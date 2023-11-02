#pragma once

#include "common.h"
#include "system.h"
#include "external/discord_game_sdk.h"

#define DISCORD_FIELD_SIZE		128

struct DiscordInstance {
	struct IDiscordCore *core;
	struct IDiscordActivityManager *activities;
	struct IDiscordApplicationManager *application;
};

enum ReturnStatus connect_to_discord(struct DiscordInstance *instance);

struct DiscordInstance* init_connect_to_discord();

enum ReturnStatus discord_callbacks(struct DiscordInstance *instance);

enum ReturnStatus set_discord_activity(
	struct DiscordInstance *inst, struct DiscordActivity *activity);

enum ReturnStatus set_discord_activity_from(
	struct DiscordInstance *inst,
	const char *front_line,
	const char *large_img_url,
	const char *large_img_txt,
	const char *small_img_url,
	const char *small_img_txt);

