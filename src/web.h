#pragma once

#define MAX_URL_SIZE	255

#include <curl/curl.h>

char* request_web_page(
	CURL *curl, const char *url, const char **headers, size_t num_headers);
