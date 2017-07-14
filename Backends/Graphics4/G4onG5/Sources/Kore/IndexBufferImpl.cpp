#include "pch.h"

#include "IndexBufferImpl.h"

#include <Kore/Graphics4/Graphics.h>

#include <Kore/Graphics5/CommandList.h>

using namespace Kore;

extern Graphics5::CommandList* commandList;

Kore::IndexBufferImpl::IndexBufferImpl(int count) : _buffer(count, true) {}

Graphics4::IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {}

Graphics4::IndexBuffer::~IndexBuffer() {}

int* Graphics4::IndexBuffer::lock() {
	return _buffer.lock();
}

void Graphics4::IndexBuffer::unlock() {
	_buffer.unlock();
	commandList->upload(&_buffer);
}

void Graphics4::IndexBuffer::_set() {
	_buffer._set();
}

int Graphics4::IndexBuffer::count() {
	return _buffer.count();;
}
