#pragma once

#include "pch.h"

#include "vertexstructure.h"

#include <kinc/backend/graphics5/vertexbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_vertex_buffer {
	VertexBuffer5Impl impl;
} kinc_g5_vertex_buffer_t;

KINC_FUNC void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory,
                                          int instanceDataStepRate);
KINC_FUNC void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer);
KINC_FUNC float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer);
KINC_FUNC float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count);
KINC_FUNC void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer);
KINC_FUNC void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count);
KINC_FUNC int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer);
KINC_FUNC int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer);

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer, int offset); // Do not call this directly, use Graphics::setVertexBuffers

#ifdef __cplusplus
}
#endif
