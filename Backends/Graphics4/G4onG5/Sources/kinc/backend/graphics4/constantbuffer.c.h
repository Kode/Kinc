#ifdef KINC_KONG

#include <kinc/graphics4/constantbuffer.h>

void kinc_g4_constant_buffer_init(kinc_g4_constant_buffer *buffer, size_t size) {
	kinc_g5_constant_buffer_init(&buffer->impl.buffer, (int)size);
}

void kinc_g4_constant_buffer_destroy(kinc_g4_constant_buffer *buffer) {
	kinc_g5_constant_buffer_destroy(&buffer->impl.buffer);
}

uint8_t *kinc_g4_constant_buffer_lock_all(kinc_g4_constant_buffer *buffer) {
	kinc_g5_constant_buffer_lock_all(&buffer->impl.buffer);
	return buffer->impl.buffer.data;
}

uint8_t *kinc_g4_constant_buffer_lock(kinc_g4_constant_buffer *buffer, size_t start, size_t size) {
	kinc_g5_constant_buffer_lock(&buffer->impl.buffer, (int)start, (int)size);
	return buffer->impl.buffer.data;
}

void kinc_g4_constant_buffer_unlock_all(kinc_g4_constant_buffer *buffer) {
	kinc_g5_constant_buffer_unlock(&buffer->impl.buffer);
}

void kinc_g4_constant_buffer_unlock(kinc_g4_constant_buffer *buffer, size_t count) {
	kinc_g5_constant_buffer_unlock(&buffer->impl.buffer);
}

size_t kinc_g4_constant_buffer_size(kinc_g4_constant_buffer *buffer) {
	return kinc_g5_constant_buffer_size(&buffer->impl.buffer);
}

#endif
