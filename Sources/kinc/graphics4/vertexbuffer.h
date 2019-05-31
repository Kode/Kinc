#pragma once

#include "vertexstructure.h"

#include <Kore/VertexBufferImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_vertex_buffer {
	Kinc_G4_VertexBufferImpl impl;
} kinc_g4_vertex_buffer_t;

typedef enum kinc_g4_usage {
	KINC_G4_USAGE_STATIC,
	KINC_G4_USAGE_DYNAMIC,
	KINC_G4_USAGE_READABLE
} kinc_g4_usage_t;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                 int instance_data_step_rate);
void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer);
float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count);
void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer);
void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count);
int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer);
int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer);

int Kinc_Internal_G4_VertexBuffer_Set(kinc_g4_vertex_buffer_t *buffer, int offset);

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count);
void kinc_g4_set_vertex_buffer(kinc_g4_vertex_buffer_t *buffer);

#ifdef __cplusplus
}
#endif
