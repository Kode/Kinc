#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

namespace {
	const int multiple = 50;
}

extern u64 frameNumber;
extern bool waitAfterNextDraw;

Kore::VertexBufferImpl::VertexBufferImpl(int count, const Graphics4::VertexStructure& structure, bool gpuMemory, int instanceDataStepRate)
    : _multiple(multiple), _buffer(count * multiple, structure, gpuMemory, instanceDataStepRate), _lastFrameNumber(0), _currentIndex(0), myCount(count) {}

Graphics4::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, Usage usage, int instanceDataStepRate)
    : VertexBufferImpl(count, structure, usage == StaticUsage, instanceDataStepRate) {}

Graphics4::VertexBuffer::~VertexBuffer() {}

void VertexBufferImpl::prepareLock() {
	/*if (frameNumber > _lastFrameNumber) {
	    _lastFrameNumber = frameNumber;
	    _currentIndex = 0;
	}
	else {*/
	++_currentIndex;
	if (_currentIndex >= _multiple - 1) {
		waitAfterNextDraw = true;
	}
	if (_currentIndex >= _multiple) {
		_currentIndex = 0;
	}
	//}
}

float* Graphics4::VertexBuffer::lock() {
	prepareLock();
	return _buffer.lock(_currentIndex * count(), count());
}

float* Graphics4::VertexBuffer::lock(int start, int count) {
	prepareLock();
	return _buffer.lock(start + _currentIndex * this->count(), count);
}

void Graphics4::VertexBuffer::unlock() {
	_buffer.unlock();
}

void Graphics4::VertexBuffer::unlock(int count) {
	_buffer.unlock(count);
}

int Graphics4::VertexBuffer::count() {
	return myCount;
}

int Graphics4::VertexBuffer::stride() {
	return _buffer.stride();
}
