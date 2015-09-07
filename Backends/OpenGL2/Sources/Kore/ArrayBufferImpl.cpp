#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include "ogl.h"

using namespace Kore;

ArrayBuffer* ArrayBufferImpl::current = nullptr;

ArrayBufferImpl::ArrayBufferImpl(int count, int structureSize, int structureCount) : mySize(count), structureSize(structureSize), structureCount(structureCount) {

}

ArrayBuffer::ArrayBuffer(int indexCount, int structureSize, int structureCount) : ArrayBufferImpl(indexCount, structureSize, structureCount) {
	glGenBuffers(1, &bufferId);
	data = new float[indexCount];
}

ArrayBuffer::~ArrayBuffer() {
	unset();
	delete[] data;
}

float* ArrayBuffer::lock() {
	return data;
}

void ArrayBuffer::unlock() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mySize * 4, data, GL_STATIC_DRAW);
}

void ArrayBuffer::set(AttributeLocation location, int divisor) {
	current = this;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	for (int i = 0; i < structureCount; i++) {
		glEnableVertexAttribArray(location.location + i);
		glVertexAttribPointer(location.location + i, structureSize, GL_FLOAT, false, 4 * structureSize * structureCount, (void*)(i * 4 * structureSize));
		glVertexAttribDivisor(location.location + i, divisor);
	}
}

void ArrayBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int ArrayBuffer::count() {
	return mySize;
}
