#include "indexbuffer.h"

#include <kinc/graphics5/indexbuffer.h>

#include <stdlib.h>

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.myCount = count;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	return NULL;
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
