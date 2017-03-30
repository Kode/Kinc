#include "pch.h"

#include "IndexBufferImpl.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

Kore::IndexBufferImpl::IndexBufferImpl(int count) : _buffer(count) {}

Graphics4::IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {}

Graphics4::IndexBuffer::~IndexBuffer() {}

int* Graphics4::IndexBuffer::lock() {
	return _buffer.lock();
}

void Graphics4::IndexBuffer::unlock() {
	_buffer.unlock();
}

void Graphics4::IndexBuffer::_set() {
	_buffer._set();
}

int Graphics4::IndexBuffer::count() {
	return _buffer.count();;
}
