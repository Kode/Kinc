#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

using namespace Kore;

Kore::VertexBufferImpl::VertexBufferImpl(int count) {}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count) {

}

VertexBuffer::~VertexBuffer() {

}

float* VertexBuffer::lock() {
	return lock(0, count());
}

float* VertexBuffer::lock(int start, int count) {
	return nullptr;
}

void VertexBuffer::unlock() {

}

int VertexBuffer::count() {
	return 0;
}

int VertexBuffer::stride() {
	return 0;
}
