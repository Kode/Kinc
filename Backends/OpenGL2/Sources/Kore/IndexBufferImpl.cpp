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
}

IndexBuffer::~IndexBuffer() {
	unset();
	delete[] data;
}

int* IndexBuffer::lock() {
	return data;
}

void IndexBuffer::unlock() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, myCount * 4, data, GL_STATIC_DRAW);
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
