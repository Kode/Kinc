#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *file;
} Kinc_FileWriter;

bool Kinc_FileWriter_Open(Kinc_FileWriter *writer, const char *filepath);
void Kinc_FileWriter_Write(Kinc_FileWriter *writer, void *data, int size);
void Kinc_FileWriter_Close(Kinc_FileWriter *writer);

#ifdef __cplusplus
}
#endif
