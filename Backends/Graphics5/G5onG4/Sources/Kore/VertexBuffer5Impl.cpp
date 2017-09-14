#include "pch.h"

#include "VertexBuffer5Impl.h"

#include <Kore/Graphics5/Graphics.h>

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

VertexBuffer5Impl* VertexBuffer5Impl::_current = nullptr;

VertexBuffer5Impl::VertexBuffer5Impl(int count) : myCount(count), myStart(0) {}

Graphics5::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, bool gpuMemory, int instanceDataStepRate) : VertexBuffer5Impl(count) {
	buffer = new Graphics4::VertexBuffer(count, structure, instanceDataStepRate);
}

Graphics5::VertexBuffer::~VertexBuffer() {
	delete buffer;
}

float* Graphics5::VertexBuffer::lock() {
	return lock(0, count());
}

float* Graphics5::VertexBuffer::lock(int start, int count) {
	return buffer->lock(start, count);
}

void Graphics5::VertexBuffer::unlock() {
	buffer->unlock();
}

int Graphics5::VertexBuffer::count() {
	return myCount;
}

int Graphics5::VertexBuffer::stride() {
	return myStride;
}
