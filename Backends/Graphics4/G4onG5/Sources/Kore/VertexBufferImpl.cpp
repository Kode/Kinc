#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

namespace {
	const int multiple = 5000;
}

Kore::VertexBufferImpl::VertexBufferImpl(int count, const Graphics4::VertexStructure& structure, int instanceDataStepRate) : _multiple(multiple), _buffer(count * multiple, structure, false, instanceDataStepRate), _currentIndex(0), myCount(count) {}

Graphics4::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count, structure, instanceDataStepRate) {}

Graphics4::VertexBuffer::~VertexBuffer() {}

float* Graphics4::VertexBuffer::lock() {
	return _buffer.lock(_currentIndex * count(), count());
}

float* Graphics4::VertexBuffer::lock(int start, int count) {
	return _buffer.lock(start + _currentIndex * this->count(), count);
}

void Graphics4::VertexBuffer::unlock() {
	_buffer.unlock();

	++_currentIndex;
	if (_currentIndex >= _multiple) {
		_currentIndex = 0;
	}
}

int Graphics4::VertexBuffer::count() {
	return myCount;
}

int Graphics4::VertexBuffer::stride() {
	return _buffer.stride();
}
