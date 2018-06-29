#include "pch.h"

#include <Kore/Graphics5/Graphics.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

Graphics5::IndexBuffer* IndexBuffer5Impl::current = nullptr;

IndexBuffer5Impl::IndexBuffer5Impl(int count) : myCount(count) {}

Graphics5::IndexBuffer::IndexBuffer(int indexCount, bool gpuMemory) : IndexBuffer5Impl(indexCount) {
	this->gpuMemory = gpuMemory;
	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLCPUCacheModeWriteCombined;
#ifdef KORE_IOS
	options |= MTLResourceStorageModeShared;
#else
	if (gpuMemory) {
		options |= MTLResourceStorageModeManaged;
	}
	else {
		options |= MTLResourceStorageModeShared;
	}
#endif
	mtlBuffer = [device newBufferWithLength:sizeof(int) * indexCount options:options];
}

Graphics5::IndexBuffer::~IndexBuffer() {
	unset();
}

int* Graphics5::IndexBuffer::lock() {
	id<MTLBuffer> buffer = mtlBuffer;
	return (int*)[buffer contents];
}

void Graphics5::IndexBuffer::unlock() {
#ifndef KORE_IOS
	if (gpuMemory) {
		id<MTLBuffer> buffer = mtlBuffer;
		NSRange range;
		range.location = 0;
		range.length = count() * 4;
		[buffer didModifyRange:range];
	}
#endif
}

void Graphics5::IndexBuffer::_set() {
	current = this;
}

void IndexBuffer5Impl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int Graphics5::IndexBuffer::count() {
	return myCount;
}
