#include "config.h"
#include "discord.h"
#include "simplyplural.h"
#include "pluralkit.h"
#include "manual.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// TODO: Host elsewhere pls
#define APP_ICON_URL 	"http://hayley.byethost33.com/fronting-64.png"

char* force_http(char *url)
{
	// Test if HTTPS
	if(url && url == strstr(url, "https://"))
	{
		char uri[DISCORD_FIELD_SIZE];
		strcpy(uri, url + 8);	// url minus the https://

		strcpy(url, "http://");
		strcat(url, uri);
	}

	return url;
}

bool get_fronter_icon(
	char *image,
	char *text,
	const struct MemberInfo *fronters,
	size_t num_fronters)
{
	assert(image && text);
	assert((num_fronters == 0) || (fronters && (num_fronters > 0)));

	if(num_fronters == 0)
		return false;

	// Use the avatar of the first fronter
	if(fronters[0].avatar_url && (strcmp(fronters[0].avatar_url, "") != 0))
	{
		strcpy(image, fronters[0].avatar_url);
		strcpy(text, fronters[0].name);
	}

	// Override avatar if a custom front w/ URL is available
	for(int i = 0; i < num_fronters; ++i)
	{
		if((fronters[i].type == EMemberType_State) 
			&& fronters[i].avatar_url)
		{
			strcpy(image, fronters[i].avatar_url);
			strcpy(text, fronters[i].name);
			break;
		}
	}

	if(strlen(image) == 0)
		return false;

	image = force_http(image);

	return true;
}

bool get_system_icon(char *image, char *text, const struct SystemInfo *system)
{
	assert(image && text);
	assert(system);

	// Use the avatar of the first fronter
	if(strcmp(system->avatar_url, "") != 0)
	{
		strcpy(image, system->avatar_url);
		strcpy(text, system->name);

		image = force_http(image);
	}

	return true;
}

enum ReturnStatus get_large_icon(
	char *large_image,
	char *large_text,
	const struct Configuration *config,
	const struct SystemInfo *system,
	const struct MemberInfo *fronters,
	size_t num_fronters)
{
	assert(large_image && large_text && config && system);

	switch(config->avatar_mode)
	{
		case EAvatarMode_Member:
		{
			get_fronter_icon(large_image, large_text, fronters, num_fronters);
		}
		break;
		case EAvatarMode_MemberSystem:
		{
			if(get_fronter_icon(
				large_image, large_text, fronters, num_fronters) == true)
			{
				break;
			}
		}
		case EAvatarMode_System:
		{
			get_system_icon(large_image, large_text, system);
		}
		break;
		case EAvatarMode_App:
			strcpy(large_image, APP_ICON_URL);
			return ReturnStatus_Ok;

		case EAvatarMode_NoAvatar:
	}

	return ReturnStatus_Ok;
}

enum ReturnStatus get_small_icon(
	char *small_image,
	char *small_text,
	const struct Configuration *config,
	const struct SystemInfo *system,
	const struct MemberInfo *fronters,
	size_t num_fronters)
{
	assert(small_image && small_text && config && system);

	switch(config->icon_mode)
	{
		case EIconMode_Member:
		{
			get_fronter_icon(small_image, small_text, fronters, num_fronters);
		}
		break;
		case EIconMode_System:
		{
			get_system_icon(small_image, small_text, system);
		}
		break;
		case EIconMode_NoIcon:
	}

	return ReturnStatus_Ok;
}

char* get_name_entry(const struct MemberInfo *member, bool show_pronouns)
{
	assert(member);
	
	if(!show_pronouns || !member->pronouns || (strlen(member->pronouns) == 0))
	{
		char *new_str = (char*) malloc(
			(strlen(member->name) + 1) * sizeof(char));
		strcpy(new_str, member->name);
		return new_str;
	}

	assert(member->pronouns);

	char* new_str = (char*) malloc(
		(strlen(member->name) + strlen(member->pronouns) + 4) * sizeof(char));

	sprintf(new_str, "%s (%s)", member->name, member->pronouns);

	return new_str;
}

enum ReturnStatus send_to_discord(
	struct DiscordInstance *discord,
	const struct Configuration *config,
	const struct SystemInfo *system,
	const struct MemberInfo *fronters,
	size_t num_fronters)
{
	assert(discord);
	assert(config);
	assert(system);

	printf("Setting Discord activity... ");

	const int last_fronter_index = num_fronters - 1;

	// Create new activity, ensure zeroed out
	struct DiscordActivity activity;
	memset((void*)&activity, 0, sizeof(activity));

	// Fill out the activity fronting line
	if(num_fronters == 0)
		strcpy(activity.details, "[Nobody]");
	else
		strcpy(activity.details, "");

	for(int i = 0; i < num_fronters; ++i)
	{
		char *name_entry = get_name_entry(&fronters[i], config->show_pronouns);
		strcat(activity.details, name_entry);
		free(name_entry);

		if((i == 0) && (num_fronters == 2))	// Headmate_1 & Headmate_2
			strcat(activity.details, " & ");
		else if(i < last_fronter_index)		// Fronter_1, Fronter_2, Fronter_3
			strcat(activity.details, ", ");
	}

	if(config->source == ESource_SimplyPlural)
		strcpy(activity.state, "(from Simply Plural)");
	else if(config->source == ESource_PluralKit)
		strcpy(activity.state, "(from PluralKit)");
	
	get_large_icon(
		activity.assets.large_image,
		activity.assets.large_text,
		config,
		system,
		fronters,
		num_fronters);
	get_small_icon(
		activity.assets.small_image,
		activity.assets.small_text,
		config,
		system,
		fronters,
		num_fronters);

	set_discord_activity(discord, &activity);

	printf("done.\r\n");

	return ReturnStatus_Ok;
}

enum ReturnStatus handle_simplyplural(
	const struct Configuration *config, struct DiscordInstance *discord)
{
	assert(config && discord);

	const int sp_ticks = (int) ceil((double)config->sp_config.polling_rate 
		/ (double)config->discord_poll_rate);

	struct SystemInfo system;
	struct SimplyPluralInstance sp;
	if(sp_init(&sp, config->sp_config.token) != ReturnStatus_Ok)
	{
		printf("Failed to initialize Simply Plural module!\r\n");
		return EXIT_FAILURE;
	}

	while(1)
	{
		if(sp_get_system(&system, &sp) == ReturnStatus_Error)
			fprintf(stderr, 
				"Error fetching system info from Simply Plural.\r\n");

		printf(
			"System Info:\r\n\tName: %s\r\n\tAvatar URL: %s\r\n",
			system.name, system.avatar_url);

		struct MemberInfo *fronters;
		const int num_fronters = sp_get_fronters(&fronters, &sp);
		if(num_fronters < 0)
		{
			fprintf(stderr, "Error fetching fronters from Simply Plural.\r\n");
			SLEEP_SECS(60);
			continue;
		}

		printf("Current Fronters:\r\n");

		const int last_front_index = num_fronters - 1;
		for(int i = 0; i < num_fronters; ++i)
		{
			printf("\t%s:\r\n", fronters[i].name); 
			printf(
				"\t\tType: %s\r\n", 
				fronters[i].type == EMemberType_State ? 
					"State" : "Headmate");
			printf("\t\tPronouns: %s\r\n", fronters[i].pronouns);
			printf("\t\tAvatar URL: %s\r\n", fronters[i].avatar_url);
		}

		if(send_to_discord(
			discord, config, &system, fronters, num_fronters)
				!= ReturnStatus_Ok)
		{
			return ReturnStatus_Error;
		}

		printf("\r\n");

		for(int i = 0; i < sp_ticks; ++i)
		{
			if(discord_callbacks(discord) != ReturnStatus_Ok)
			{
				if(discord_status() != DiscordResult_Ok)
				{
					fprintf(stderr, "Discord Error: %s\r\n", discord_error());
					return ReturnStatus_Ok;
				}
			}
			SLEEP_SECS(config->discord_poll_rate);
		}
	}

	return ReturnStatus_Ok;
}

enum ReturnStatus handle_pluralkit(
	const struct Configuration *config, struct DiscordInstance *discord)
{
	assert(config && discord);

	const int pk_ticks = (int) ceil((double)config->pk_config.polling_rate 
		/ (double)config->discord_poll_rate);

	struct SystemInfo system;
	struct PluralKitInstance pk;
	if(pk_init(&pk, config->pk_config.token) != ReturnStatus_Ok)
	{
		fprintf(stderr, "Failed to initialize PluralKit module!\r\n");
		return EXIT_FAILURE;
	}

	while(1)
	{
		if(pk_get_system(&system, &pk) == ReturnStatus_Error)
			fprintf(stderr, 
				"Error fetching system info from PluralKit.\r\n");

		printf(
			"System Info:\r\n\tName: %s\r\n\tAvatar URL: %s\r\n",
			system.name, system.avatar_url);

		struct MemberInfo *fronters;
		const int num_fronters = pk_get_fronters(&fronters, &pk);
		if(num_fronters < 0)
		{
			fprintf(stderr, "Error fetching fronters from PluralKit.\r\n");
			SLEEP_SECS(60);
			continue;
		}

		printf("Current Fronters:\r\n");

		const int last_front_index = num_fronters - 1;
		for(int i = 0; i < num_fronters; ++i)
		{
			printf("\t%s:\r\n", fronters[i].name); 
			printf(
				"\t\tType: %s\r\n", 
				fronters[i].type == EMemberType_State ? 
					"State" : "Headmate");
			printf("\t\tPronouns: %s\r\n", fronters[i].pronouns);
			printf("\t\tAvatar URL: %s\r\n", fronters[i].avatar_url);
		}

		if(send_to_discord(
			discord, config, &system, fronters, num_fronters)
				!= ReturnStatus_Ok)
		{
			return ReturnStatus_Error;
		}

		printf("\r\n");

		for(int i = 0; i < pk_ticks; ++i)
		{
			if(discord_callbacks(discord) != ReturnStatus_Ok)
			{
				if(discord_status() != DiscordResult_Ok)
				{
					fprintf(stderr, "Discord Error: %s\r\n", discord_error());
					return ReturnStatus_Ok;
				}
			}
			SLEEP_SECS(config->discord_poll_rate);
		}
	}

	return ReturnStatus_Ok;
}


struct Arguments {
	enum ETask {
		ETask_Normal,

		ETask_ServiceEnable,
		ETask_ServiceDisable,
		ETask_ServiceMode,

		ETask_AddFront,
		ETask_RemoveFront,
		ETask_AddMember,
		ETask_RemoveMember,
		ETask_ModifyMember
	} task;
};

struct Arguments parse_program_arguments(int argc, const char **argv)
{
	struct Arguments args;
	memset((void*)&args, 0, sizeof(args));

	args.task = ETask_Normal;
	if(argc > 1)
	{
		if(!strcmp(argv[1], "enable-service"))
			args.task = ETask_ServiceEnable;
		else if(!strcmp(argv[1], "disable-service"))
			args.task = ETask_ServiceDisable;
		else if(!strcmp(argv[1], "service"))
			args.task = ETask_ServiceMode;

		else if(!strcmp(argv[1], "add-front"))
			args.task = ETask_AddFront;
		else if(!strcmp(argv[1], "remove-front"))
			args.task = ETask_RemoveFront;
		else if(!strcmp(argv[1], "add-member"))
			args.task = ETask_AddMember;
		else if(!strcmp(argv[1], "remove-member"))
			args.task = ETask_RemoveMember;
		else if(!strcmp(argv[1], "edit-member"))
			args.task = ETask_ModifyMember;
	}

	return args;
}

int main(int argc, const char **argv)
{
	const struct Arguments args = parse_program_arguments(argc, argv);

	if(args.task == ETask_ServiceEnable)
	{
		enum ReturnStatus ret = enable_service();
		if(ret == ReturnStatus_Ok)
		{
			printf("Service Mode enabled.\r\n");
			return EXIT_SUCCESS;
		}
		else
		{
			printf("Could not enable service mode!\r\n");
			return EXIT_FAILURE;
		}
	}
	else if(args.task == ETask_ServiceDisable)
	{
		enum ReturnStatus ret = disable_service();
		if(ret == ReturnStatus_Ok)
		{
			printf("Service Mode disabled.\r\n");
			return EXIT_SUCCESS;
		}
		else
		{
			printf("Could not disable service mode!\r\n");
			return EXIT_FAILURE;
		}
	}

	// First, load the configuration file
	struct Configuration config;
	memset((void*)&config, 0, sizeof(config));
	if(load_config(&config) == ReturnStatus_Error)
	{
		fprintf(stderr, "Error loading config file!\r\n");
		return EXIT_FAILURE;
	}

	bool is_initial_run = true;
	while(is_initial_run || (args.task == ETask_ServiceMode))
	{
		is_initial_run = false;

		// If we have configured the program, attempt to connect to Discord
		printf("Waiting for Discord... ");
		struct DiscordInstance *discord = NULL;
		while(!discord)
		{
			discord = init_connect_to_discord();
			if(!discord)
				SLEEP_SECS(config.discord_poll_rate);
		}

		printf("connected.\r\n");

		// We have a Discord connection, so now query source for information
		struct SystemInfo system;
		switch(config.source)
		{
			case ESource_SimplyPlural:
			{
				printf("Using Simply Plural.\r\n");
				
				if(handle_simplyplural(&config, discord) != ReturnStatus_Ok)
					return EXIT_FAILURE;
			}
			break;
			case ESource_PluralKit:
			{
				printf("Using PluralKit.\r\n");

				if(handle_pluralkit(&config, discord) != ReturnStatus_Ok)
					return EXIT_FAILURE;
			}
			break;
			case ESource_Manual:
			{
				printf("ERROR: Manual fronting is not yet implemented!\r\n");
			}
			break;
		}

		destroy_discord(discord);
	}

	return EXIT_SUCCESS;
}
