#include "config.h"
#include "system.h"
#include "discord.h"
#include "simplyplural.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// TODO: Host elsewhere pls
#define APP_ICON_URL 	"http://hayley.byethost33.com/fronting-64.png"

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
	if(strcmp(fronters[0].avatar_url, "") != 0)
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
			strcpy(large_image, system->avatar_url);
			strcpy(large_text, system->name);
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
			strcpy(small_image, system->avatar_url);
			strcpy(small_text, system->name);
		}
		break;
		case EIconMode_NoIcon:
	}

	return ReturnStatus_Ok;
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
		strcpy(activity.details, "[Nobody Fronting]");
	else
		strcpy(activity.details, "");

	for(int i = 0; i < num_fronters; ++i)
	{
		strcat(activity.details, fronters[i].name);

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
		struct MemberInfo *fronters;
		const int num_fronters = sp_get_fronters(&fronters, &sp);
		if(num_fronters < 0)
		{
			printf("Error fetching fronters from Simply Plural.\r\n");
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

		for(int i = 0; i < sp_ticks; ++i)
		{
			discord_callbacks(discord);
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
		ETask_ServiceMode
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
	}

	return args;
}

int main(int argc, const char **argv)
{
	const struct Arguments args = parse_program_arguments(argc, argv);

	if(args.task == ETask_ServiceEnable)
	{
		int ret = enable_service();
		if(ret == ReturnStatus_Ok)
			return EXIT_SUCCESS;
		else
			return EXIT_FAILURE;
	}
	else if(args.task == ETask_ServiceDisable)
	{
		int ret = disable_service();
		if(ret == ReturnStatus_Ok)
			return EXIT_SUCCESS;
		else
			return EXIT_FAILURE;
	}

	// First, load the configuration file
	struct Configuration config;
	memset((void*)&config, 0, sizeof(config));
	if(load_config(&config) == ReturnStatus_Error)
	{
		printf("Error loading config file!\r\n");
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
			}
			break;
			case ESource_Manual:
			{
				printf("ERROR: Manual fronting is not yet implemented!\r\n");
			}
			break;
		}
	}

	return EXIT_SUCCESS;
}
