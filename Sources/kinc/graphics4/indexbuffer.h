#pragma once

#include <Kore/IndexBufferImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_index_buffer {
	Kinc_G4_IndexBufferImpl impl;
} kinc_g4_index_buffer_t;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count);
void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer);
int *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer);
void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer);
int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer);

void Kinc_Internal_G4_IndexBuffer_Set(kinc_g4_index_buffer_t *buffer);

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer);

#ifdef __cplusplus
}
#endif
