#pragma once

#include "common.h"

#define SIMPLY_PLURAL_TOKEN_SIZE	65

struct SimplyPluralConfig {
	char token[SIMPLY_PLURAL_TOKEN_SIZE];
	unsigned int polling_rate;
};

#define PLURALKIT_TOKEN_SIZE		65

struct PluralKitConfig {
	char token[PLURALKIT_TOKEN_SIZE];
	unsigned int polling_rate;
};

struct ManualConfig {
	char filepath[PATH_MAX];
};

enum ESource {
	ESource_Manual,
	ESource_SimplyPlural,
	ESource_PluralKit
};

enum EAvatarMode {
	EAvatarMode_Member,
	EAvatarMode_System,
	EAvatarMode_MemberSystem,
	EAvatarMode_App,
	EAvatarMode_NoAvatar
};

enum EIconMode {
	EIconMode_NoIcon,
	EIconMode_Member,
	EIconMode_System
};

struct Configuration {
	enum ESource source;

	enum EAvatarMode avatar_mode;
	enum EIconMode icon_mode;

	unsigned int discord_poll_rate;

	bool show_pronouns;
	bool show_fronting_time;

	struct PluralKitConfig pk_config;
	struct SimplyPluralConfig sp_config;
	struct ManualConfig mnl_config;
};

enum ReturnStatus load_config(struct Configuration *config_out);

enum ReturnStatus create_config();

enum ReturnStatus enable_service();

enum ReturnStatus disable_service();

