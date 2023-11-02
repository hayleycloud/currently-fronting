#include "simplyplural.h"
#include "web.h"
#include "external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define SP_API_URI		"https://api.apparyllis.com/v1/"
#define SP_SRV_URI		"https://serve.apparyllis.com/"

void make_sp_api_url(char* url_out, const char* endpoint)
{
	strcpy(url_out, SP_API_URI);
	strcat(url_out, endpoint);
}

void make_sp_serve_url(char* url_out, const char* uri)
{
	strcpy(url_out, SP_SRV_URI);
	strcat(url_out, uri);
}

enum ReturnStatus sp_init(struct SimplyPluralInstance *sp, const char *token)
{
	assert(sp && token);

	curl_global_init(CURL_GLOBAL_DEFAULT);

	sp->curl = curl_easy_init();
	if(!sp->curl)
		return ReturnStatus_Error;

	strcpy(sp->auth, "Authorization: ");
	strcat(sp->auth, token);

	return ReturnStatus_Ok;
}

void sp_destroy(struct SimplyPluralInstance *sp)
{
	assert(sp);
	curl_easy_cleanup(sp->curl);
	curl_global_cleanup();
}

cJSON* fetch_fronters_data(const struct SimplyPluralInstance *sp)
{
	assert(sp);

	const char *headers = sp->auth;

	char url[MAX_URL_SIZE];
	make_sp_api_url(url, "fronters");

	return cJSON_Parse(request_web_page(sp->curl, url, &headers, 1));
}

cJSON* fetch_member_data(
	const char *systemid,
	const char *memberid,
	const struct SimplyPluralInstance *sp)
{
	assert(systemid && memberid);
	assert(sp);

	const char *headers = sp->auth;

	char endpoint[MAX_URL_SIZE];
	char url[MAX_URL_SIZE];

	sprintf(endpoint, "member/%s/%s", systemid, memberid);
	make_sp_api_url(url, endpoint);

	return cJSON_Parse(request_web_page(sp->curl, url, &headers, 1));
}

cJSON* fetch_customfront_data(
	const char *systemid,
	const char *memberid,
	const struct SimplyPluralInstance *sp)
{
	assert(systemid && memberid);
	assert(sp);

	const char *headers = sp->auth;

	char endpoint[MAX_URL_SIZE];
	char url[MAX_URL_SIZE];

	sprintf(endpoint, "customFront/%s/%s", systemid, memberid);
	make_sp_api_url(url, endpoint);

	return cJSON_Parse(request_web_page(sp->curl, url, &headers, 1));
}

enum ReturnStatus get_uid_from(char *uid_out, cJSON *fronters_data)
{
	assert(uid_out && fronters_data);

	const cJSON *content = NULL, *uid = NULL, *item = NULL;

	if(!cJSON_IsArray(fronters_data))
		return ReturnStatus_Error;
	if(cJSON_GetArraySize(fronters_data) <= 0)
		return ReturnStatus_Error;

	item = cJSON_GetArrayItem(fronters_data, 0);
	if(!cJSON_IsObject(item))
		return ReturnStatus_Error;

	content = cJSON_GetObjectItemCaseSensitive(item, "content");
	if(!cJSON_IsObject(content))
		return ReturnStatus_Error;

	uid = cJSON_GetObjectItemCaseSensitive(content, "uid");
	if(!cJSON_IsString(uid))
		return ReturnStatus_Error;

	strcpy(uid_out, uid->valuestring);

	return ReturnStatus_Ok;
}

int get_fronter_ids_from(char ***memberids_out, cJSON *fronters_data)
{
	assert(memberids_out && fronters_data);

	const cJSON *content, *memberid, *item = NULL;

	if(!cJSON_IsArray(fronters_data))
		return -1;

	const int num_fronters = cJSON_GetArraySize(fronters_data);

	*memberids_out = (char**) calloc(num_fronters, sizeof(char**));

	int front_index = 0;
	cJSON_ArrayForEach(item, fronters_data)
	{
		if(!cJSON_IsObject(item))
			continue;

		content = cJSON_GetObjectItemCaseSensitive(item, "content");
		if(!cJSON_IsObject(content))
			continue;

		memberid = cJSON_GetObjectItemCaseSensitive(content, "member");
		if(!cJSON_IsString(memberid))
			continue;

		(*memberids_out)[front_index] =
			(char*) malloc(sizeof(char) * (strlen(memberid->valuestring) + 1));
		strcpy((*memberids_out)[front_index], memberid->valuestring);

		++front_index;
	}

	return num_fronters;
}

void free_fronter_ids(int size, char **member_ids)
{
	for(int index = 0; index < size; ++index)
		free(member_ids[index]);
	free(member_ids);
}

enum ReturnStatus get_member_str_from(
	char **data_out, cJSON *member_data, const char *field_name)
{
	assert(data_out && member_data && field_name);

	const cJSON *content, *data;
   
	if(!cJSON_IsObject(member_data))
		return ReturnStatus_Error;

	content = cJSON_GetObjectItemCaseSensitive(member_data, "content");
	if(!cJSON_IsObject(content))
		return ReturnStatus_Error;

	data = cJSON_GetObjectItemCaseSensitive(content, field_name);
	if(!cJSON_IsString(data))
		return ReturnStatus_Error;

	size_t data_len = strlen(data->valuestring);
	*data_out = (char*) malloc(sizeof(char) * (data_len + 1));
	strcpy(*data_out, data->valuestring);

	return ReturnStatus_Ok;
}

char* avatar_uuid_to_url(const char *systemid, char *uuid)
{
	assert(systemid && uuid);

	const size_t url_len = 
		strlen(SP_SRV_URI) + strlen(systemid) + strlen(uuid) + 10;

	char* url = (char*) malloc(sizeof(char) * url_len);

	sprintf(url, "%savatars/%s/%s", SP_SRV_URI, systemid, uuid);

	free(uuid);

	return url;
}

char* handle_avatar_location(const char *systemid, char *avatar)
{
	assert(systemid);

	// No avatar in profile
	if(avatar == NULL)
		return NULL;

	// Avatar is a UUID
	if(strspn(avatar, "1234567890abcdef-") == strlen(avatar))
		return avatar_uuid_to_url(systemid, avatar);
	
	// Avatar is a URL
	return avatar;
}

enum ReturnStatus get_member_info(
	struct MemberInfo *member,
	const char *systemid,
	const char *member_id,
	const struct SimplyPluralInstance *sp)
{
	assert(member);
	assert(systemid && member_id);
	assert(sp);

	cJSON *member_data = fetch_member_data(systemid, member_id, sp);
	if(!member_data)
		return ReturnStatus_Error;

	member->type = EMemberType_Headmate;
	
	// Require a name for our members!
	if(get_member_str_from(&member->name, member_data, "name")
		!= ReturnStatus_Ok)
		return ReturnStatus_Error;

	// Pronouns are optional, so let NULLs sit
	get_member_str_from(&member->pronouns, member_data, "pronouns");

	// Avatars are optional, so let NULLs sit
	get_member_str_from(&member->avatar_url, member_data, "avatarUuid");
	member->avatar_url = handle_avatar_location(systemid, member->avatar_url);

	return ReturnStatus_Ok;
}

enum ReturnStatus get_customfront_info(
	struct MemberInfo *member,
	const char *systemid,
	const char *member_id,
	const struct SimplyPluralInstance *sp)
{
	assert(member);
	assert(systemid && member_id);
	assert(sp);

	cJSON *member_data = fetch_customfront_data(systemid, member_id, sp);
	if(!member_data)
		return ReturnStatus_Error;

	member->type = EMemberType_State;
	
	// Require a name for our members!
	if(get_member_str_from(&member->name, member_data, "name")
		!= ReturnStatus_Ok)
		return ReturnStatus_Error;

	// Avatars are optional, so let NULLs sit
	get_member_str_from(&member->avatar_url, member_data, "avatarUuid");
	member->avatar_url = handle_avatar_location(systemid, member->avatar_url);

	return ReturnStatus_Ok;
}

const int sp_get_fronters(
	struct MemberInfo **fronters_out, const struct SimplyPluralInstance *sp)
{
	assert(sp);
	assert(sp->curl && sp->auth);
	assert(fronters_out);

	char systemid[SP_SYSTEMID_SIZE];

	cJSON *fronters_data = fetch_fronters_data(sp);
	if(!fronters_data)
		return -1;

	if(get_uid_from(systemid, fronters_data) != ReturnStatus_Ok)
		return -1;

	char **fronter_ids;
	int num_fronters = get_fronter_ids_from(&fronter_ids, fronters_data);
	if(num_fronters < 0)
		return -1;

	*fronters_out = (struct MemberInfo*) calloc(
		num_fronters, sizeof(struct MemberInfo));
	for(int i = 0; i < num_fronters; ++i)
	{
		if(get_member_info(&(*fronters_out)[i], systemid, fronter_ids[i], sp)
			!= ReturnStatus_Ok)
		{
			if(get_customfront_info(
				&(*fronters_out)[i], systemid, fronter_ids[i], sp)
					!= ReturnStatus_Ok)
			{
				printf(
					"Could not fetch data for member %s.\r\n", fronter_ids[i]);
			}
		}
	}

	free_fronter_ids(num_fronters, fronter_ids);

	return num_fronters;
}

void sp_free_fronters(struct MemberInfo* fronters, const int num_fronters)
{
	free(fronters);
}
