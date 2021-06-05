#include "ogl.h"
#include <kinc/backend/graphics4/shader.h>
#include <kinc/backend/graphics4/vertexbuffer.h>

#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <assert.h>
#include <stdlib.h>

extern kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer;
static kinc_g4_vertex_buffer_t *currentVertexBuffer = NULL;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
void *glesVertexAttribDivisor;
#endif

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int vertexCount, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                int instanceDataStepRate) {
	buffer->impl.myCount = vertexCount;
	buffer->impl.instanceDataStepRate = instanceDataStepRate;
#ifndef NDEBUG
	buffer->impl.initialized = false;
#endif
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g4_vertex_element_t element = structure->elements[i];
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_COLOR:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT1:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			buffer->impl.myStride += 2 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			buffer->impl.myStride += 3 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			buffer->impl.myStride += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			buffer->impl.myStride += 4 * 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			buffer->impl.myStride += 2 * 2;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			buffer->impl.myStride += 4 * 2;
			break;
		case KINC_G4_VERTEX_DATA_NONE:
			break;
		}
	}
	buffer->impl.structure = *structure;
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

	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	glBufferData(GL_ARRAY_BUFFER, buffer->impl.myStride * buffer->impl.myCount, NULL, buffer->impl.usage);
	glCheckErrors();
	buffer->impl.data = (float *)malloc(vertexCount * buffer->impl.myStride);
}

void Kinc_Internal_G4_VertexBuffer_Unset(kinc_g4_vertex_buffer_t *buffer);

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	Kinc_Internal_G4_VertexBuffer_Unset(buffer);
	glDeleteBuffers(1, &buffer->impl.bufferId);
	free(buffer->impl.data);
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	buffer->impl.sectionStart = 0;
	buffer->impl.sectionSize = buffer->impl.myCount * buffer->impl.myStride;
	return buffer->impl.data;
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	buffer->impl.sectionStart = start * buffer->impl.myStride;
	buffer->impl.sectionSize = count * buffer->impl.myStride;
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	return (float *)&u8data[buffer->impl.sectionStart];
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	glBufferSubData(GL_ARRAY_BUFFER, buffer->impl.sectionStart, buffer->impl.sectionSize, u8data + buffer->impl.sectionStart);
	glCheckErrors();
#ifndef NDEBUG
	buffer->impl.initialized = true;
#endif
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	uint8_t *u8data = (uint8_t *)buffer->impl.data;
	glBufferSubData(GL_ARRAY_BUFFER, buffer->impl.sectionStart, count * buffer->impl.myStride, u8data + buffer->impl.sectionStart);
	glCheckErrors();
#ifndef NDEBUG
	buffer->impl.initialized = true;
#endif
}

int Kinc_G4_Internal_SetVertexAttributes(kinc_g4_vertex_buffer_t *buffer, int offset);

int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset) {
	assert(buffer->impl.initialized); // Vertex Buffer is used before lock/unlock was called
	int offsetoffset = Kinc_G4_Internal_SetVertexAttributes(buffer, offset);
	if (Kinc_Internal_CurrentIndexBuffer != NULL) {
		kinc_internal_g4_index_buffer_set(Kinc_Internal_CurrentIndexBuffer);
	}
	return offsetoffset;
}

void Kinc_Internal_G4_VertexBuffer_Unset(kinc_g4_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer) {
		currentVertexBuffer = NULL;
	}
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

#if !defined(KORE_OPENGL_ES) || (defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
static bool attribDivisorUsed = false;
#endif

int Kinc_G4_Internal_SetVertexAttributes(kinc_g4_vertex_buffer_t *buffer, int offset) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();

	int internaloffset = 0;
	int actualIndex = 0;
	for (int index = 0; index < buffer->impl.structure.size; ++index) {
		kinc_g4_vertex_element_t element = buffer->impl.structure.elements[index];
		int size = 0;
		GLenum type = GL_FLOAT;
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_COLOR:
			size = 4;
			type = GL_UNSIGNED_BYTE;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT1:
			size = 1;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			size = 2;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			size = 3;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			size = 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			size = 16;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			size = 2;
			type = GL_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			size = 4;
			type = GL_SHORT;
			break;
		case KINC_G4_VERTEX_DATA_NONE:
			break;
		}
		if (size > 4) {
			int subsize = size;
			int addonOffset = 0;
			while (subsize > 0) {
				glEnableVertexAttribArray(offset + actualIndex);
				glCheckErrors();
				glVertexAttribPointer(offset + actualIndex, 4, type, false, buffer->impl.myStride, (void *)(int64_t)(internaloffset + addonOffset));
				glCheckErrors();
#ifndef KORE_OPENGL_ES
				if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
					attribDivisorUsed = true;
					glVertexAttribDivisor(offset + actualIndex, buffer->impl.instanceDataStepRate);
					glCheckErrors();
				}
#endif
#if (defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
				if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
					attribDivisorUsed = true;
					((void (*)(GLuint, GLuint))glesVertexAttribDivisor)(offset + actualIndex, buffer->impl.instanceDataStepRate);
					glCheckErrors();
				}
#endif
				subsize -= 4;
				addonOffset += 4 * 4;
				++actualIndex;
			}
		}
		else {
			glEnableVertexAttribArray(offset + actualIndex);
			glCheckErrors();
			glVertexAttribPointer(offset + actualIndex, size, type, type == GL_FLOAT ? false : true, buffer->impl.myStride, (void *)(int64_t)internaloffset);
			glCheckErrors();
#ifndef KORE_OPENGL_ES
			if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
				attribDivisorUsed = true;
				glVertexAttribDivisor(offset + actualIndex, buffer->impl.instanceDataStepRate);
				glCheckErrors();
			}
#endif
#if (defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
			if (attribDivisorUsed || buffer->impl.instanceDataStepRate != 0) {
				attribDivisorUsed = true;
				((void (*)(GLuint, GLuint))glesVertexAttribDivisor)(offset + actualIndex, buffer->impl.instanceDataStepRate);
				glCheckErrors();
			}
#endif
			++actualIndex;
		}
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_COLOR:
			internaloffset += 4 * 1;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT1:
			internaloffset += 4 * 1;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			internaloffset += 4 * 2;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			internaloffset += 4 * 3;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			internaloffset += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			internaloffset += 4 * 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			internaloffset += 2 * 2;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			internaloffset += 2 * 4;
			break;
		case KINC_G4_VERTEX_DATA_NONE:
			break;
		}
	}
	int count = 16 - offset;
	for (int index = actualIndex; index < count; ++index) {
		glDisableVertexAttribArray(offset + index);
		glCheckErrors();
	}
	return actualIndex;
}
