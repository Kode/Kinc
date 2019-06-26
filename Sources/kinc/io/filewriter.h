#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *file;
	const char* filename;
	bool mounted;
} kinc_file_writer_t;

bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath);
void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size);
void kinc_file_writer_close(kinc_file_writer_t *writer);

#ifdef __cplusplus
}
#endif
