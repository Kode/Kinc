#ifdef KINC_KONG

#include <kinc/graphics4/constantbuffer.h>

void kinc_g4_constant_buffer_init(kinc_g4_constant_buffer *buffer, size_t size) {
	buffer->impl.size = size;
	buffer->impl.last_start = 0;
	buffer->impl.last_size = size;

	buffer->impl.data = malloc(size);

	buffer->impl.buffer = 0;
	glGenBuffers(1, &buffer->impl.buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer->impl.buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void kinc_g4_constant_buffer_destroy(kinc_g4_constant_buffer *buffer) {
	free(buffer->impl.data);
	buffer->impl.data = NULL;

	glDeleteBuffers(1, &buffer->impl.buffer);
	buffer->impl.buffer = 0;
}

uint8_t *kinc_g4_constant_buffer_lock_all(kinc_g4_constant_buffer *buffer) {
	return kinc_g4_constant_buffer_lock(buffer, 0, kinc_g4_constant_buffer_size(buffer));
}

uint8_t *kinc_g4_constant_buffer_lock(kinc_g4_constant_buffer *buffer, size_t start, size_t size) {
	buffer->impl.last_start = start;
	buffer->impl.last_size = size;

	uint8_t *data = (uint8_t *)buffer->impl.data;
	return &data[start];
}

void kinc_g4_constant_buffer_unlock_all(kinc_g4_constant_buffer *buffer) {
	kinc_g4_constant_buffer_unlock(buffer, buffer->impl.last_size);
}

void kinc_g4_constant_buffer_unlock(kinc_g4_constant_buffer *buffer, size_t count) {
	glBindBuffer(GL_UNIFORM_BUFFER, buffer->impl.buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, buffer->impl.last_start, buffer->impl.last_size, buffer->impl.data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

size_t kinc_g4_constant_buffer_size(kinc_g4_constant_buffer *buffer) {
	return buffer->impl.size;
}

#endif
