#ifdef KINC_KONG

#include <kinc/graphics4/constantbuffer.h>

#include "Direct3D9.h"

void kinc_g4_constant_buffer_init(kinc_g4_constant_buffer *buffer, size_t size) {
	buffer->impl.size = size;
	buffer->impl.last_start = 0;
	buffer->impl.last_size = size;
}

void kinc_g4_constant_buffer_destroy(kinc_g4_constant_buffer *buffer) {}

uint8_t *kinc_g4_constant_buffer_lock_all(kinc_g4_constant_buffer *buffer) {
	return NULL;
}

uint8_t *kinc_g4_constant_buffer_lock(kinc_g4_constant_buffer *buffer, size_t start, size_t size) {
	buffer->impl.last_start = start;
	buffer->impl.last_size = size;

	return NULL;
}

void kinc_g4_constant_buffer_unlock_all(kinc_g4_constant_buffer *buffer) {}

void kinc_g4_constant_buffer_unlock(kinc_g4_constant_buffer *buffer, size_t count) {}

size_t kinc_g4_constant_buffer_size(kinc_g4_constant_buffer *buffer) {
	return buffer->impl.size;
}

#endif
