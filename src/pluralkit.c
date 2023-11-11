#include "pluralkit.h"
#include "web.h"
#include "external/cJSON.h"
#include <stdlib.h>
#include <string.h>

#define PK_API_URI		"https://api.pluralkit.me/v2/"

void make_pk_api_url(char* url_out, const char* endpoint)
{
	strcpy(url_out, PK_API_URI);
	strcat(url_out, endpoint);
}

enum ReturnStatus pk_init(struct PluralKitInstance *pk, const char *token)
{
	assert(pk && token);

	curl_global_init(CURL_GLOBAL_DEFAULT);

	pk->curl = curl_easy_init();
	if(!pk->curl)
		return ReturnStatus_Error;

	strcpy(pk->auth, "Authorization: ");
	strcat(pk->auth, token);

	return ReturnStatus_Ok;
}

void pk_destroy(struct PluralKitInstance *pk)
{
	assert(pk);
	curl_easy_cleanup(pk->curl);
	curl_global_cleanup();
}

cJSON* pk_fetch_system_data(const struct PluralKitInstance *pk)
{
	assert(pk);

	const char *headers = pk->auth;

	char url[MAX_URL_SIZE];
	make_pk_api_url(url, "systems/@me");

	return cJSON_Parse(request_web_page(pk->curl, url, &headers, 1));
}

cJSON* pk_fetch_fronters_data(const struct PluralKitInstance *pk)
{
	assert(pk);

	const char *headers = pk->auth;

	char url[MAX_URL_SIZE];
	make_pk_api_url(url, "systems/@me/fronters");

	return cJSON_Parse(request_web_page(pk->curl, url, &headers, 1));
}

enum ReturnStatus pk_get_member_str_from(
	char **data_out, const cJSON *member_data, const char *field_name)
{
	assert(data_out && member_data && field_name);

	const cJSON *data;
   
	if(!cJSON_IsObject(member_data))
		return ReturnStatus_Error;

	data = cJSON_GetObjectItemCaseSensitive(member_data, field_name);
	if(!cJSON_IsString(data))
		return ReturnStatus_Error;

	size_t data_len = strlen(data->valuestring);
	*data_out = (char*) malloc(sizeof(char) * (data_len + 1));
	strcpy(*data_out, data->valuestring);

	return ReturnStatus_Ok;
}

enum ReturnStatus get_fronter_from(
	struct MemberInfo *fronter_out, const cJSON *fronter_data)
{
	assert(fronter_out && fronter_data);

	fronter_out->type = EMemberType_Headmate;

	if(pk_get_member_str_from(&fronter_out->name, fronter_data, "name")
		!= ReturnStatus_Ok)
		return ReturnStatus_Error;

	pk_get_member_str_from(&fronter_out->pronouns, fronter_data, "pronouns");
	pk_get_member_str_from(&fronter_out->avatar_url, fronter_data, "avatar_url");

	return ReturnStatus_Ok;
}

const int get_fronters_from(
	struct MemberInfo **fronters_out, const cJSON *fronters_data)
{
	assert(fronters_out && fronters_data);

	cJSON *members = cJSON_GetObjectItemCaseSensitive(fronters_data, "members");

	if(!cJSON_IsArray(members))
		return -1;

	const int num_fronters = cJSON_GetArraySize(members);

	*fronters_out = (struct MemberInfo*) calloc(
		num_fronters, sizeof(struct MemberInfo));

	int front_index = 0;
	cJSON *item = NULL;
	cJSON_ArrayForEach(item, members)
	{
		if(!cJSON_IsObject(item))
			continue;

		get_fronter_from(&(*fronters_out)[front_index], item);

		++front_index;
	}

	return num_fronters;
}

const int pk_get_fronters(
	struct MemberInfo **fronters_out, const struct PluralKitInstance *pk)
{
	assert(fronters_out && pk);
	assert(pk->curl);
	
	cJSON *fronters_data = pk_fetch_fronters_data(pk);
	if(!fronters_data)
		return -1;

	const int num_fronters = get_fronters_from(fronters_out, fronters_data);

	cJSON_Delete(fronters_data);

	return num_fronters;
}

enum ReturnStatus pk_get_system_str_from(
	char **data_out, const cJSON *system_data, const char *field_name)
{
	assert(data_out && system_data && field_name);

	const cJSON *data;
   
	if(!cJSON_IsObject(system_data))
		return ReturnStatus_Error;

	data = cJSON_GetObjectItemCaseSensitive(system_data, field_name);
	if(!cJSON_IsString(data))
		return ReturnStatus_Error;

	size_t data_len = strlen(data->valuestring);
	*data_out = (char*) malloc(sizeof(char) * (data_len + 1));
	strcpy(*data_out, data->valuestring);

	return ReturnStatus_Ok;
}

enum ReturnStatus pk_get_system(
	struct SystemInfo *system, const struct PluralKitInstance *pk)
{
	assert(system && pk);
	assert(pk->curl);
	
	cJSON *system_data = pk_fetch_system_data(pk);
	if(!system_data)
		return ReturnStatus_Error;

	if(pk_get_system_str_from(&system->name, system_data, "name")
		!= ReturnStatus_Ok)
	{
		cJSON_Delete(system_data);
		return ReturnStatus_Error;
	}

	pk_get_system_str_from(&system->avatar_url, system_data, "avatar_url");
	
	cJSON_Delete(system_data);

	return ReturnStatus_Ok;
}

