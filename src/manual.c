#include "manual.h"
#include "external/cJSON.h"
#include <string.h>
#include <stdlib.h>

cJSON* mnl_load_file(struct ManualInstance *inst)
{
	assert(inst);
	assert(strlen(inst->filepath) > 0);

	char *data;
	if(read_txt_file(&data, inst->filepath) == 0)
		return NULL;

	cJSON *json = cJSON_Parse(data);
	free(data);

	return json;
}

enum EManualStatus mnl_save_file(struct ManualInstance *inst, cJSON *data)
{
	assert(inst);
	assert(strlen(inst->filepath) > 0);

	char *data_str = cJSON_Print(data);

	if(write_txt_file(data_str, inst->filepath) != ReturnStatus_Ok)
		return EManualStatus_Error;

	free(data_str);

	return EManualStatus_Ok;
}

enum EManualStatus mnl_create_file(const char *filepath)
{
	const char empty_system_json[] = 
		"{\r\n\
		\t\"system\": {\r\n\
		\t\t\"name\": \"\",\r\n\
		\t\t\"avatar_url\": \"\"\r\n\
		\t},\r\n\
		\t\"members\": [\r\n\
		\t],\r\n\
		\t\"fronters\": [\r\n\
		\t]\r\n\
		}";
	
	if(write_txt_file(empty_system_json, filepath) != ReturnStatus_Ok)
		return EManualStatus_Error;

	return EManualStatus_Ok;
}

enum EManualStatus mnl_init(
	struct ManualInstance *inst, const char *filepath)
{
	memset((void*)inst, 0, sizeof(struct ManualInstance));

	strcpy(inst->filepath, filepath);

	FILE* f = fopen(filepath, "r");
	if(f == NULL)
	{
		if(mnl_create_file(filepath) == EManualStatus_Error)
			return EManualStatus_Error;
	}

	return EManualStatus_Ok;
}
