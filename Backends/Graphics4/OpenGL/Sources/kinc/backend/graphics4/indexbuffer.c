#include "ogl.h"

#include <kinc/graphics4/indexbuffer.h>

#include <stdlib.h>

kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer = NULL;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format, kinc_g4_usage_t usage) {
	buffer->impl.myCount = count;
	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
	buffer->impl.data = (int *)malloc(count * sizeof(int));
	if (format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT) {
		buffer->impl.shortData = (uint16_t *)malloc(count * sizeof(uint16_t));
	}
	else
		buffer->impl.shortData = NULL;

	switch (usage) {
	case KINC_G4_USAGE_STATIC:
		buffer->impl.usage = GL_STATIC_DRAW;
		break;
	case KINC_G4_USAGE_DYNAMIC:
		buffer->impl.usage = GL_DYNAMIC_DRAW;
		break;
	case KINC_G4_USAGE_READABLE:
		buffer->impl.usage = GL_DYNAMIC_DRAW;
		break;
	}
}

void Kinc_Internal_IndexBufferUnset(kinc_g4_index_buffer_t *buffer) {
	if (Kinc_Internal_CurrentIndexBuffer == buffer) {
		Kinc_Internal_CurrentIndexBuffer = NULL;
	}
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	Kinc_Internal_IndexBufferUnset(buffer);
	glDeleteBuffers(1, &buffer->impl.bufferId);
	free(buffer->impl.data);
	if (buffer->impl.shortData != NULL) {
		free(buffer->impl.shortData);
	}
}

int *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.data;
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer) {
	if (buffer->impl.shortData != NULL) {
		for (int i = 0; i < buffer->impl.myCount; ++i) {
			buffer->impl.shortData[i] = (uint16_t)buffer->impl.data[i];
		}
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	if (buffer->impl.shortData != NULL) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.myCount * 2, buffer->impl.shortData, buffer->impl.usage);
		glCheckErrors();
	}
	else {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.myCount * 4, buffer->impl.data, buffer->impl.usage);
		glCheckErrors();
	}
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
