cmake_minimum_required(VERSION 3.15)
project(
	CurrentlyFronting 
	VERSION 1.0
	DESCRIPTION "Currently Fronting"
	HOMEPAGE_URL "https://github.com/hayleycloud/currently-fronting"
	LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(discord_lib SHARED IMPORTED)
if(FOR_WINDOWS)	# We can do better checking here!

else()
	#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib/discord_game_sdk.so 
	set_target_properties(discord_lib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/discord_game_sdk.so)
endif()

add_executable(CurrentlyFronting 
	src/main.c 
	src/common.c
	src/config.c
	src/web.c
	src/discord.c
	src/pluralkit.c
	src/simplyplural.c
	src/external/cJSON.c)
target_link_libraries(CurrentlyFronting curl discord_lib m)
set_target_properties(CurrentlyFronting PROPERTIES OUTPUT_NAME fronting)

install(TARGETS CurrentlyFronting RUNTIME DESTINATION bin)
