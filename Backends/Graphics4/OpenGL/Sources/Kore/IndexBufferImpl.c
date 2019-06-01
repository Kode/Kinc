#include "pch.h"

#include "ogl.h"

#include <kinc/graphics4/indexbuffer.h>

#include <stdlib.h>

kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer = NULL;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count) {
	buffer->impl.myCount = count;
	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
	buffer->impl.data = (int*)malloc(count * sizeof(int));
#if defined(KORE_ANDROID) || defined(KORE_PI)
	shortData = (uint16_t*)malloc(count * sizeof(uint16_t));
#endif
}

void Kinc_Internal_IndexBufferUnset(kinc_g4_index_buffer_t *buffer) {
	if (Kinc_Internal_CurrentIndexBuffer == buffer) {
		Kinc_Internal_CurrentIndexBuffer = NULL;
	}
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	Kinc_Internal_IndexBufferUnset(buffer);
	free(buffer->impl.data);
}

int *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.data;
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer) {
#if defined(KORE_ANDROID) || defined(KORE_PI)
	for (int i = 0; i < buffer->impl.myCount; ++i) {
		buffer->impl.shortData[i] = (uint16_t)buffer->impl.data[i];
	}
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
#if defined(KORE_ANDROID) || defined(KORE_PI)
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 2, shortData, GL_STATIC_DRAW);
	glCheckErrors();
#else
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.myCount * 4, buffer->impl.data, GL_STATIC_DRAW);
	glCheckErrors();
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glCheckErrors();
}

void kinc_internal_g4_index_buffer_set(kinc_g4_index_buffer_t *buffer) {
	Kinc_Internal_CurrentIndexBuffer = buffer;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
