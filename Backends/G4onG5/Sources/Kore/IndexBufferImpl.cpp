#include "pch.h"

#include "IndexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

using namespace Kore;

Kore::IndexBufferImpl::IndexBufferImpl(int count) {}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {

}

IndexBuffer::~IndexBuffer() {

}

int* IndexBuffer::lock() {
	return nullptr;
}

void IndexBuffer::unlock() {

}

void IndexBuffer::_set() {

}

int IndexBuffer::count() {
	return 0;
}
