#include "pch.h"

#include "ogl.h"

#include <Kinc/Graphics4/IndexBuffer.h>

#include <stdlib.h>

Kinc_G4_IndexBuffer *Kinc_Internal_CurrentIndexBuffer = NULL;

void Kinc_G4_IndexBuffer_Create(Kinc_G4_IndexBuffer *buffer, int count) {
	buffer->impl.myCount = count;
	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
	buffer->impl.data = (int*)malloc(count * sizeof(int));
#if defined(KORE_ANDROID) || defined(KORE_PI)
	shortData = (uint16_t*)malloc(count * sizeof(uint16_t));
#endif
}

void Kinc_Internal_IndexBufferUnset(Kinc_G4_IndexBuffer *buffer) {
	if (Kinc_Internal_CurrentIndexBuffer == buffer) {
		Kinc_Internal_CurrentIndexBuffer = NULL;
	}
}

void Kinc_G4_IndexBuffer_Destroy(Kinc_G4_IndexBuffer *buffer) {
	Kinc_Internal_IndexBufferUnset(buffer);
	free(buffer->impl.data);
}

int *Kinc_G4_IndexBuffer_Lock(Kinc_G4_IndexBuffer *buffer) {
	return buffer->impl.data;
}

void Kinc_G4_IndexBuffer_Unlock(Kinc_G4_IndexBuffer *buffer) {
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

void Kinc_Internal_G4_IndexBuffer_Set(Kinc_G4_IndexBuffer *buffer) {
	Kinc_Internal_CurrentIndexBuffer = buffer;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
}

int Kinc_G4_IndexBuffer_Count(Kinc_G4_IndexBuffer *buffer) {
	return buffer->impl.myCount;
}
