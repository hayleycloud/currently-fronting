#include "discord.h"
#include <stdlib.h>
#include <string.h>

#define DISCORD_CLIENT_ID	1164726159339176016

void DISCORD_CALLBACK on_oauth2_token(
	void *data,
    enum EDiscordResult result,
    struct DiscordOAuth2Token *token)
{
    if (result == DiscordResult_Ok) {
        printf("OAuth2 token: %s\r\n", token->access_token);
    }
    else {
        printf("GetOAuth2Token failed with %d\r\n", (int)result);
    }
}

void DISCORD_CALLBACK update_activity_callback(
	void *data, enum EDiscordResult result)
{
    assert(result == DiscordResult_Ok);
}

enum ReturnStatus connect_to_discord(struct DiscordInstance *inst)
{
	assert(inst);

	struct DiscordCreateParams params;
	DiscordCreateParamsSetDefault(&params);
	params.client_id = DISCORD_CLIENT_ID;
	params.flags = DiscordCreateFlags_Default;
	params.event_data = inst;
	
	if(DiscordCreate(DISCORD_VERSION, &params, &inst->core) != DiscordResult_Ok)
		return ReturnStatus_Error;

	inst->activities = inst->core->get_activity_manager(inst->core);
	inst->application = inst->core->get_application_manager(inst->core);

	inst->application->get_oauth2_token(
		inst->application, inst, on_oauth2_token);

	return ReturnStatus_Ok;
}

struct DiscordInstance* init_connect_to_discord()
{
	struct DiscordInstance *inst = 
		(struct DiscordInstance*) malloc(sizeof(struct DiscordInstance));

	if(connect_to_discord(inst) != ReturnStatus_Ok)
		return NULL;

	return inst;
}

enum ReturnStatus discord_callbacks(struct DiscordInstance *instance)
{
	assert(instance);

	if(instance->core->run_callbacks(instance->core) != DiscordResult_Ok)
		return ReturnStatus_Error;

	return ReturnStatus_Ok;
}

enum ReturnStatus set_discord_activity(
	struct DiscordInstance *inst, struct DiscordActivity *activity)
{
	inst->activities->update_activity(
		inst->activities, activity, inst, update_activity_callback);
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

