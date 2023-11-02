#pragma once

struct SystemInfo {
	char *name;
	char *avatar_url;
};

enum EMemberType {
	EMemberType_Headmate,
	EMemberType_State
};

struct MemberInfo {
	enum EMemberType type;
	char *name;
	char *pronouns;
	char *avatar_url;
};
