#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

//#define 

#if defined(__APPLE__)
#define PLATFORM 	MACOS
#elif defined(__unix) || defined(__unix__)
#define PLATFORM	UNIX
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM 	WINDOWS
#endif

#if PLATFORM == UNIX
#include <unistd.h>
#define SLEEP_SECS(s)	sleep(s)
#elif PLATFORM == WINDOWS
#define SLEEP_SECS(s)	Sleep(s * 1000)
#endif

enum ReturnStatus {
	ReturnStatus_Ok = 0,
	ReturnStatus_Error
};

size_t file_sizeof(FILE *file);
size_t read_txt_file(char **data_out, const char *path);
enum ReturnStatus write_txt_file(char *data_in, const char *path);
