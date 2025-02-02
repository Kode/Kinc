#include "buffer_functions.h"

#include <kope/graphics5/buffer.h>

void kope_metal_buffer_set_name(kope_g5_buffer *buffer, const char *name) {}

void kope_metal_buffer_destroy(kope_g5_buffer *buffer) {}

void *kope_metal_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	return (void *)[metal_buffer contents];
}

void *kope_metal_buffer_lock_all(kope_g5_buffer *buffer) {
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	return (void *)[metal_buffer contents];
}

void *kope_metal_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	uint8_t *data = (uint8_t *)[metal_buffer contents];
	return &data[offset];
}

void *kope_metal_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->metal.buffer;
	uint8_t *data = (uint8_t *)[metal_buffer contents];
	return &data[offset];
}

void kope_metal_buffer_unlock(kope_g5_buffer *buffer) {}
