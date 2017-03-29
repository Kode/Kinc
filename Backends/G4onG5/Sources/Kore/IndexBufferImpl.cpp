#include "pch.h"

#include "IndexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

using namespace Kore;

Kore::IndexBufferImpl::IndexBufferImpl(int count) : _buffer(count) {}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {}

IndexBuffer::~IndexBuffer() {}

int* IndexBuffer::lock() {
	return _buffer.lock();
}

void IndexBuffer::unlock() {
	_buffer.unlock();
}

void IndexBuffer::_set() {
	_buffer._set();
}

int IndexBuffer::count() {
	return _buffer.count();;
}
