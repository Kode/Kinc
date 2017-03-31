#include "pch.h"

#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

Graphics4::IndexBuffer* IndexBufferImpl::current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {}

Graphics4::IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {
	glGenBuffers(1, &bufferId);
	glCheckErrors();
	data = new int[indexCount];
#if defined(KORE_ANDROID) || defined(KORE_PI)
	shortData = new u16[indexCount];
#endif
}

Graphics4::IndexBuffer::~IndexBuffer() {
	unset();
	delete[] data;
}

int* Graphics4::IndexBuffer::lock() {
	return data;
}

void Graphics4::IndexBuffer::unlock() {
#if defined(KORE_ANDROID) || defined(KORE_PI)
	for (int i = 0; i < myCount; ++i) shortData[i] = (u16)data[i];
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	glCheckErrors();
#if defined(KORE_ANDROID) || defined(KORE_PI)
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 2, shortData, GL_STATIC_DRAW);
	glCheckErrors();
#else
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 4, data, GL_STATIC_DRAW);
	glCheckErrors();
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glCheckErrors();
}

void Graphics4::IndexBuffer::_set() {
	current = this;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	glCheckErrors();
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int Graphics4::IndexBuffer::count() {
	return myCount;
}
