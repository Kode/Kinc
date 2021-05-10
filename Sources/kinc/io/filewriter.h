#pragma once

#include <kinc/global.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *file;
	const char *filename;
	bool mounted;
} kinc_file_writer_t;

KINC_FUNC bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath);
KINC_FUNC void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size);
KINC_FUNC void kinc_file_writer_close(kinc_file_writer_t *writer);

#ifdef __cplusplus
}
#endif
