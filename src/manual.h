#pragma once

#include "common.h"

enum EManualStatus {
	EManualStatus_Ok,
	EManualStatus_Error
};

struct ManualInstance {
	char filepath[PATH_MAX];
};

enum EManualStatus mnl_init(
	struct ManualInstance *inst, const char *filepath);

const int mnl_get_fronters(
	struct MemberInfo **fronters, 
	struct ManualInstance *inst);

enum EManualStatus mnl_add_to_front(
	struct ManualInstance *inst,
	const char *name);

enum EManualStatus mnl_remove_from_front(
	struct ManualInstance *inst,
	const char *name);

enum EManualStatus mnl_add_member(
	struct ManualInstance *inst,
	const char *name, 
	const char *pronouns, 
	const char *avatar_url);

enum EManualStatus mnl_remove_member(
	struct ManualInstance *inst,
	const char *name);

enum EManualStatus mnl_modify_member(
	struct ManualInstance *inst,
	const char *name,
	const char *new_name,
	const char *new_pronouns,
	const char *new_avatar_url);
