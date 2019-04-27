#include "pch.h"

#include "IndexBuffer5Impl.h"

#include <Kinc/Graphics5/IndexBuffer.h>

kinc_g5_index_buffer_t *kinc_g5_internal_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.myCount = count;
	kinc_g4_index_buffer_init(&buffer->impl.buffer, count);
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	kinc_g4_index_buffer_destroy(&buffer->impl.buffer);
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_lock(&buffer->impl.buffer);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock(&buffer->impl.buffer);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_internal_current_index_buffer = buffer;
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
