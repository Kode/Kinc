#include "pch.h"

#include <Kore/Graphics5/Graphics.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

Graphics5::IndexBuffer* IndexBuffer5Impl::current = nullptr;
const int more = 10;

IndexBuffer5Impl::IndexBuffer5Impl(int count) : myCount(count) {}

Graphics5::IndexBuffer::IndexBuffer(int indexCount, bool gpuMemory) : IndexBuffer5Impl(indexCount) {
	id<MTLDevice> device = getMetalDevice();
	mtlBuffer = [device newBufferWithLength:sizeof(int) * indexCount * more options:MTLResourceOptionCPUCacheModeDefault];
	index = -1;
}

Graphics5::IndexBuffer::~IndexBuffer() {
	unset();
}

int* Graphics5::IndexBuffer::lock() {
	++index;
	if (index >= more) index = 0;

	id<MTLBuffer> buffer = mtlBuffer;
	int* ints = (int*)[buffer contents];
	return &ints[index * myCount];
	// return (int*)[buffer contents];
}

void Graphics5::IndexBuffer::unlock() {}

void Graphics5::IndexBuffer::_set() {
	current = this;
}

void IndexBuffer5Impl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBuffer5Impl::offset() {
	return index * myCount * sizeof(int);
}

int Graphics5::IndexBuffer::count() {
	return myCount;
}
