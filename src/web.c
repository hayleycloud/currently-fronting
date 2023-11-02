#include "web.h"
#include <stdlib.h>
#include <string.h>

#define USER_AGENT_STRING	"Plural-Frontline/1.0 libcurl-agent/1.0"

struct memory {
	char *response;
	size_t size;
};

size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata)
{
    size_t total_size = size * nmemb;
	struct memory *mem = (struct memory *)userdata;

	char *ptr = realloc(mem->response, mem->size + total_size + 1);
	if(ptr == NULL)
		return 0;	// Out of memory!

	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, total_size);
	mem->size += total_size;
	mem->response[mem->size] = 0;

    return total_size;
}

char* request_web_page(
	CURL *curl, const char *url, const char **headers, size_t num_headers)
{
	CURLcode res;
	struct curl_slist *list = NULL;
	struct memory chunk = {0};

	curl_easy_setopt(curl, CURLOPT_URL, url);

	if(headers && num_headers > 0)
	{
		for(size_t i = 0; i < num_headers; ++i)
			list = curl_slist_append(list, headers[i]);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	}

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_STRING);
	
	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		//fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return NULL;
	}

	if(headers && num_headers > 0)
		curl_slist_free_all(list);
	curl_easy_reset(curl);

	return chunk.response;
}

