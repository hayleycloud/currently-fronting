#pragma once

#include "common.h"
#include <curl/curl.h>

#define SP_AUTH_REQUEST_SIZE	80
#define SP_SYSTEMID_SIZE		30

struct SimplyPluralInstance {
	CURL *curl;
	char auth[SP_AUTH_REQUEST_SIZE];
	//char systemid[SP_SYSTEMID_SIZE];
};

enum ReturnStatus sp_init(struct SimplyPluralInstance *sp, const char *token);

void sp_destroy(struct SimplyPluralInstance *sp);

enum ReturnStatus sp_get_system(
	struct SystemInfo *system, const struct SimplyPluralInstance *sp);

const int sp_get_fronters(
	struct MemberInfo **fronters_out, const struct SimplyPluralInstance *sp);
