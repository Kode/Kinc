#pragma once

#include "pch.h"

#include <kinc/backend/graphics5/indexbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_index_buffer {
	IndexBuffer5Impl impl;
} kinc_g5_index_buffer_t;

KINC_FUNC void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory);
KINC_FUNC void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer);
KINC_FUNC int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer);
KINC_FUNC void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer);
KINC_FUNC int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer);

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer);

#ifdef __cplusplus
}
#endif
