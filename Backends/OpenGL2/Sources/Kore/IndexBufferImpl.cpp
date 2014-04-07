#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include "ogl.h"

using namespace Kore;

IndexBuffer* IndexBufferImpl::current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {
	glGenBuffers(1, &bufferId);
	data = new int[indexCount];
#ifdef SYS_ANDROID
	shortData = new u16[indexCount];
#endif
}

IndexBuffer::~IndexBuffer() {
	unset();
	delete[] data;
}

int* IndexBuffer::lock() {
	return data;
}

void IndexBuffer::unlock() {
#ifdef SYS_ANDROID
	for (int i = 0; i < myCount; ++i) shortData[i] = (u16)data[i];
#endif
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
#ifdef SYS_ANDROID
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 2, shortData, GL_STATIC_DRAW);
#else
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 4, data, GL_STATIC_DRAW);
#endif
}

void IndexBuffer::set() {
	current = this;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBuffer::count() {
	return myCount;
}
