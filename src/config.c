#include "config.h"
#include "external/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CONFIG_PATH_UNIX	"/.config"
#define PLUFRONT_CONFIG_PATH_UNIX	"currently-fronting/config.json"

int get_config_path(char *path_out)
{
	assert(path_out);

#if PLATFORM == UNIX
	const char *xdg_config_home = getenv("XDK_CONFIG_HOME");
	if(xdg_config_home)
		strcpy(path_out, xdg_config_home);
	else
	{
		const char *home_dir = getenv("HOME");
		if(!home_dir)
			return 1;
		strcpy(path_out, home_dir);
		strcat(path_out, DEFAULT_CONFIG_PATH_UNIX);
	}
	strcat(path_out, "/");
	strcat(path_out, PLUFRONT_CONFIG_PATH_UNIX);

	return 0;
#endif
}

cJSON* load_config_json(const char *path)
{
	assert(path);

	char *data;
	if(read_txt_file(&data, path) == 0)
		return NULL;

	return cJSON_Parse(data);
}

#define SOURCE_ID_PLURALKIT			"pluralkit"
#define SOURCE_ID_SIMPLYPLURAL		"simply_plural"
#define SOURCE_ID_MANUAL			"manual"

enum ReturnStatus 
read_config_source(struct Configuration *config_out, const cJSON *data)
{
	assert(config_out && data);

	const cJSON *source = cJSON_GetObjectItemCaseSensitive(data, "source");
	if(source && cJSON_IsString(source))
	{
		if(!strcmp(source->valuestring, SOURCE_ID_PLURALKIT))
		{
			config_out->source = ESource_PluralKit;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(source->valuestring, SOURCE_ID_SIMPLYPLURAL))
		{
			config_out->source = ESource_SimplyPlural;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(source->valuestring, SOURCE_ID_MANUAL))
		{
			config_out->source = ESource_Manual;
			return ReturnStatus_Ok;
		}
		else
			return ReturnStatus_Error;
	} 
	else 
		return ReturnStatus_Error;
}

#define AVATARMODE_ID_MEMBER		"member"
#define AVATARMODE_ID_SYSTEM		"system"
#define AVATARMODE_ID_MEMBERSYSTEM	"member_sys"
#define AVATARMODE_ID_APP			"app"
#define AVATARMODE_ID_NOAVATAR		""

enum ReturnStatus read_config_avatar_mode(
	struct Configuration *config_out, const cJSON *data)
{
	assert(config_out && data);

	const cJSON *mode = cJSON_GetObjectItemCaseSensitive(data, "avatar_mode");
	if(mode && cJSON_IsString(mode))
	{
		if(!strcmp(mode->valuestring, AVATARMODE_ID_MEMBER))
		{
			config_out->avatar_mode = EAvatarMode_Member;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(mode->valuestring, AVATARMODE_ID_SYSTEM))
		{
			config_out->avatar_mode = EAvatarMode_System;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(mode->valuestring, AVATARMODE_ID_MEMBERSYSTEM))
		{
			config_out->avatar_mode = EAvatarMode_MemberSystem;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(mode->valuestring, AVATARMODE_ID_APP))
		{
			config_out->avatar_mode = EAvatarMode_App;
			return ReturnStatus_Ok;
		}
		else if(!strcmp(mode->valuestring, AVATARMODE_ID_NOAVATAR))
		{
			config_out->avatar_mode = EAvatarMode_NoAvatar;
			return ReturnStatus_Ok;
		}
		else
			return ReturnStatus_Error;
	} 
	else 
		return ReturnStatus_Error;
}

enum ReturnStatus read_config_show_pronouns(
	struct Configuration *config_out, const cJSON *data)
{
	assert(config_out && data);

	const cJSON *show_pronouns = cJSON_GetObjectItemCaseSensitive(
		data, "show_pronouns");
	if(show_pronouns && cJSON_IsBool(show_pronouns))
	{
		config_out->show_pronouns = cJSON_IsTrue(show_pronouns);
		return ReturnStatus_Ok;
	} 
	else 
		return ReturnStatus_Error;
}

enum ReturnStatus read_pluralkit_settings(
	struct Configuration *config_out, const cJSON *data)
{
	assert(config_out && data);

	if(!cJSON_IsObject(data))
	{
		fprintf(stderr, "Config Error: Invalid JSON root object.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *pk_data = cJSON_GetObjectItemCaseSensitive(
		data, "pluralkit");
	if(!cJSON_IsObject(pk_data))
	{
		fprintf(stderr, "Config Error: 'pluralkit' not an object.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *token = cJSON_GetObjectItemCaseSensitive(pk_data, "token");
	if(!cJSON_IsString(token))
	{
		fprintf(stderr, 
			"Config Error: 'pluralkit/token' not a valid string.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *poll_data = cJSON_GetObjectItemCaseSensitive(
		pk_data, "poll_rate");
	if(!cJSON_IsNumber(poll_data))
	{
		fprintf(stderr,
			"Config Error: 'pluralkit/poll_rate' not an integer.\r\n");
		return ReturnStatus_Error;
	}
	else
		config_out->pk_config.polling_rate = poll_data->valueint;

	strncpy(
		config_out->pk_config.token,
		token->valuestring,
		sizeof(config_out->pk_config.token));

	return ReturnStatus_Ok;
}

enum ReturnStatus read_simplyplural_settings(
	struct Configuration *config_out, const cJSON *data)
{
	assert(config_out && data);

	if(!cJSON_IsObject(data))
	{
		fprintf(stderr, "Config Error: Invalid JSON root object.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *sp_data = cJSON_GetObjectItemCaseSensitive(
		data, "simply_plural");
	if(!cJSON_IsObject(sp_data))
	{
		fprintf(stderr, "Config Error: 'simply_plural' not an object.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *token = cJSON_GetObjectItemCaseSensitive(sp_data, "token");
	if(!cJSON_IsString(token))
	{
		fprintf(stderr, 
			"Config Error: 'simply_plural/token' not a valid string.\r\n");
		return ReturnStatus_Error;
	}

	const cJSON *poll_data = cJSON_GetObjectItemCaseSensitive(
		sp_data, "poll_rate");
	if(!cJSON_IsNumber(poll_data))
	{
		fprintf(stderr, 
			"Config Error: 'simply_plural/poll_rate' not an integer.\r\n");
		return ReturnStatus_Error;
	}
	else
		config_out->sp_config.polling_rate = poll_data->valueint;

	strncpy(
		config_out->sp_config.token, 
		token->valuestring, 
		sizeof(config_out->sp_config.token));

	return ReturnStatus_Ok;
}

enum ReturnStatus load_config(struct Configuration *config_out)
{
	char config_path[FILENAME_MAX];
	get_config_path(config_path);

	cJSON *file_data = load_config_json(config_path);
	if(!file_data)
	{
		fprintf(stderr, "Config Error: Could not read %s\r\n", config_path);
		return ReturnStatus_Error;
	}

	const cJSON *core_data = cJSON_GetObjectItemCaseSensitive(file_data, "core");
	if(!core_data)
	{
		fprintf(stderr, "Config Error: No 'core' object found.\r\n");
		cJSON_Delete(file_data);
		return ReturnStatus_Error;
	}

	if(read_config_source(config_out, core_data) == ReturnStatus_Error)
	{
		fprintf(stderr, 
			"Config Error: 'core/source' must be either %s, %s, or %s.\r\n",
			SOURCE_ID_SIMPLYPLURAL, SOURCE_ID_PLURALKIT, SOURCE_ID_MANUAL);
		cJSON_Delete(file_data);
		return ReturnStatus_Error;
	}

	if(read_config_avatar_mode(config_out, core_data) == ReturnStatus_Error)
	{
		fprintf(stderr, 
			"Config Error: 'core/avatar_mode' must be either "
			"%s, %s, %s, %s, or empty.\r\n",
			AVATARMODE_ID_MEMBERSYSTEM,
			AVATARMODE_ID_MEMBER,
			AVATARMODE_ID_SYSTEM,
			AVATARMODE_ID_APP);
		cJSON_Delete(file_data);
		return ReturnStatus_Error;
	}

	if(read_config_show_pronouns(config_out, core_data) == ReturnStatus_Error)
	{
		fprintf(stderr,
			"Config Error: 'core/show_pronouns' must be true or false.\r\n");
		cJSON_Delete(file_data);
		return ReturnStatus_Error;
	}

	const cJSON *poll_data = cJSON_GetObjectItemCaseSensitive(
		core_data, "discord_poll_rate");
	if(!cJSON_IsNumber(poll_data))
	{
		fprintf(stderr, 
			"Config Error: 'core/discord_poll_rate' not a number!\r\n");
		cJSON_Delete(file_data);
		return ReturnStatus_Error;
	}
	else
		config_out->discord_poll_rate = poll_data->valueint;
	
	if(config_out->source == ESource_SimplyPlural)
	{
		if(read_simplyplural_settings(config_out, file_data) != ReturnStatus_Ok)
		{
			cJSON_Delete(file_data);
			return ReturnStatus_Error;
		}
	}
	else if(config_out->source == ESource_PluralKit)
	{
		if(read_pluralkit_settings(config_out, file_data) != ReturnStatus_Ok)
		{
			cJSON_Delete(file_data);
			return ReturnStatus_Error;
		}
	}

	cJSON_Delete(file_data);

	return ReturnStatus_Ok;
}

enum ReturnStatus create_config()
{
	char config_path[FILENAME_MAX];
	get_config_path(config_path);

	const char initial_config[] = 
		"{\r\n\
		\t\"core\": {\r\n\
		\t\t\"show_pronouns\": false,\r\n\
		\t\t\"source\": \"simply_plural\",\r\n\
		\t\t\"avatar_mode\": \"member_sys\",\r\n\
		\t\t\"icon_mode\": \"\",\r\n\
		\t\t\"discord_poll_rate\": \"5\"\r\n\
		\t},\r\n\
		\t\"pluralkit\": {\r\n\
		\t\t\"token\": \"\",\r\n\
		\t\t\"poll_rate\": \"15\"\r\n\
		\t},\r\n\
		\t\"simply_plural\": {\r\n\
		\t\t\"token\": \"\",\r\n\
		\t\t\"poll_rate\": \"15\"\r\n\
		\t}\r\n\
		}";

	return write_txt_file(initial_config, config_path);
}

enum ReturnStatus enable_service()
{
#if PLATFORM == UNIX
	if(system("sudo systemctl daemon-reload") < 0)
		return ReturnStatus_Error;

	if(system("sudo systemctl start currently-fronting.service") < 0)
		return ReturnStatus_Error;

	if(system("sudo systemctl enable currently-fronting.service") < 0)
		return ReturnStatus_Error;
#endif

	return ReturnStatus_Ok;
}

enum ReturnStatus disable_service()
{
#if PLATFORM == UNIX
	if(system("sudo systemctl disable currently-fronting.service") < 0)
		return ReturnStatus_Error;
#endif

	return ReturnStatus_Ok;
}
