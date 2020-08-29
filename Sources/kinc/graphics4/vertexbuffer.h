#pragma once

#include "vertexstructure.h"

#include <Kore/VertexBufferImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_vertex_buffer {
	kinc_g4_vertex_buffer_impl_t impl;
} kinc_g4_vertex_buffer_t;

typedef enum kinc_g4_usage { KINC_G4_USAGE_STATIC, KINC_G4_USAGE_DYNAMIC, KINC_G4_USAGE_READABLE } kinc_g4_usage_t;

KINC_FUNC void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                          int instance_data_step_rate);
KINC_FUNC void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer);
KINC_FUNC float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer);
KINC_FUNC float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count);
KINC_FUNC void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer);
KINC_FUNC void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count);
KINC_FUNC int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer);
KINC_FUNC int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer);

KINC_FUNC int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset);

KINC_FUNC void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count);
KINC_FUNC void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer);

#ifdef __cplusplus
}
#endif
