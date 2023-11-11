#include "discord.h"
#include <stdlib.h>
#include <string.h>

#define DISCORD_CLIENT_ID	1164726159339176016

struct DiscordError {
	enum EDiscordResult code;
	const char* msg;
};

const struct DiscordError discord_errors[] = {
	{ DiscordResult_Ok, NULL },
	{ DiscordResult_NotRunning, "Discord not running." },
	{ DiscordResult_ServiceUnavailable, "Discord appears to be down." },
	{ DiscordResult_OAuth2Error, "Failed OAuth2 authentication." },
	{ DiscordResult_InvalidVersion, "Invalid Discord SDK version." },
	{ DiscordResult_NotAuthenticated, "Authentication failure." },
	{ DiscordResult_InvalidDataUrl, "Invalid data URL specified." }
};

enum EDiscordResult g_DiscordStatus = DiscordResult_Ok;

void set_discord_status(enum EDiscordResult status)
{
	g_DiscordStatus = status;
}

enum EDiscordResult discord_status()
{
	for(unsigned int i = 0; 
		i < (sizeof(discord_errors) / sizeof(struct DiscordError));
		++i)
	{
		if(discord_errors[i].code == g_DiscordStatus)
			return discord_errors[i].code;
	}

	return DiscordResult_Ok;
}

const char* discord_error()
{
	for(unsigned int i = 0; 
		i < (sizeof(discord_errors) / sizeof(struct DiscordError));
		++i)
	{
		if(discord_errors[i].code == g_DiscordStatus)
			return discord_errors[i].msg;
	}

	return NULL;
}

void DISCORD_CALLBACK on_oauth2_token(
	void *data,
    enum EDiscordResult result,
    struct DiscordOAuth2Token *token)
{
    if (result == DiscordResult_Ok) {
        printf("OAuth2 successful.\r\n");
    }
    else {
        printf("OAuth2 failed!\r\n");
    }
}

void DISCORD_CALLBACK update_activity_callback(
	void *data, enum EDiscordResult result)
{
	if(result != DiscordResult_Ok)
	{
		//printf("Discord Error: %s\r\n", (const char*) data);
	}
}

enum ReturnStatus connect_to_discord(struct DiscordInstance *inst)
{
	assert(inst);

	struct DiscordCreateParams params;
	DiscordCreateParamsSetDefault(&params);
	params.client_id = DISCORD_CLIENT_ID;
	params.flags = DiscordCreateFlags_NoRequireDiscord;
	params.event_data = inst;
	
	enum EDiscordResult result = 
		DiscordCreate(DISCORD_VERSION, &params, &inst->core);
	set_discord_status(result);
	if(result != DiscordResult_Ok)
		return ReturnStatus_Error;

	inst->activities = inst->core->get_activity_manager(inst->core);
	inst->application = inst->core->get_application_manager(inst->core);

	inst->application->get_oauth2_token(
		inst->application, inst, on_oauth2_token);

	set_discord_status(DiscordResult_Ok);

	return ReturnStatus_Ok;
}

struct DiscordInstance* init_connect_to_discord()
{
	struct DiscordInstance *inst = 
		(struct DiscordInstance*) malloc(sizeof(struct DiscordInstance));

	if(connect_to_discord(inst) != ReturnStatus_Ok)
	{
		//printf("failed: %s\r\n", discord_status());
		free(inst);
		return NULL;
	}

	return inst;
}

void destroy_discord(struct DiscordInstance *instance)
{
	assert(instance && instance->core);
	instance->core->destroy(instance->core);
	free(instance);
}

enum ReturnStatus discord_callbacks(struct DiscordInstance *instance)
{
	assert(instance);

	enum EDiscordResult result = instance->core->run_callbacks(instance->core);
	set_discord_status(result);
	if(result != DiscordResult_Ok)
		return ReturnStatus_Error;
	else
		return ReturnStatus_Ok;
}

enum ReturnStatus set_discord_activity(
	struct DiscordInstance *inst, struct DiscordActivity *activity)
{
	inst->activities->update_activity(
		inst->activities, activity, inst, update_activity_callback);
	set_discord_status(DiscordResult_Ok);
	return ReturnStatus_Ok;
}

enum ReturnStatus set_discord_activity_from(
	struct DiscordInstance *inst,
	const char *front_line,
	const char *large_img_url,
	const char *large_img_txt,
	const char *small_img_url,
	const char *small_img_txt)
{
	assert(inst);

	struct DiscordActivity activity;
	memset(&activity, 0, sizeof(activity));
	strcpy(activity.details, front_line);

	if(large_img_url)
	{
		strcpy(activity.assets.large_image, large_img_url);
		strcpy(activity.assets.large_text, large_img_txt);
	}

	if(small_img_url)
	{
		strcpy(activity.assets.small_image, small_img_url);
		strcpy(activity.assets.small_text, small_img_txt);
	}

	return ReturnStatus_Ok;
}

