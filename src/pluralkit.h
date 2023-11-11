#pragma once

#include "common.h"
#include <curl/curl.h>

#define PK_AUTH_REQUEST_SIZE	80
#define PK_SYSTEMID_SIZE		6

struct PluralKitInstance {
	CURL *curl;
	char auth[PK_AUTH_REQUEST_SIZE];
	//char systemid[PK_SYSTEMID_SIZE];
};

enum ReturnStatus pk_init(struct PluralKitInstance *pk, const char *token);

void pk_destroy(struct PluralKitInstance *pk);

enum ReturnStatus pk_get_system(
	struct SystemInfo *system, const struct PluralKitInstance *pk);

const int pk_get_fronters(
	struct MemberInfo **fronters_out, const struct PluralKitInstance *pk);
