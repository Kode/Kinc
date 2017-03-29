#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

using namespace Kore;

Kore::VertexBufferImpl::VertexBufferImpl(int count, const VertexStructure& structure, int instanceDataStepRate) : _buffer(count, structure, instanceDataStepRate) {}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count, structure, instanceDataStepRate) {}

VertexBuffer::~VertexBuffer() {}

float* VertexBuffer::lock() {
	return _buffer.lock();
}

float* VertexBuffer::lock(int start, int count) {
	return _buffer.lock(start, count);
}

void VertexBuffer::unlock() {
	_buffer.unlock();
}

int VertexBuffer::count() {
	return _buffer.count();
}

int VertexBuffer::stride() {
	return _buffer.stride();
}
