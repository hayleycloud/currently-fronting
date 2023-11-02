#include "common.h"
#include <stdlib.h>
#include <string.h>

size_t file_sizeof(FILE *file)
{
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

size_t read_txt_file(char **data_out, const char *path)
{
	FILE *file = fopen(path, "r");
	if(!file)
		return 0;

	size_t size = file_sizeof(file);
	*data_out = (char*)malloc(sizeof(char) * size);

	fread((void*)*data_out, sizeof(char), size, file);

	return size;
}

enum ReturnStatus write_txt_file(char *data_in, const char *path)
{
	FILE *file = fopen(path, "w");
	if(!file)
		return ReturnStatus_Error;

	fwrite((const void*)data_in, sizeof(char), strlen(data_in), file);

	return ReturnStatus_Ok;
}
