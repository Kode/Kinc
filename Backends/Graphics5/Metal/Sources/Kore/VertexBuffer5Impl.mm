#include "pch.h"

#include "Shader5Impl.h"
#include "VertexBuffer5Impl.h"

#include <Kore/Graphics5/Graphics.h>
#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

Graphics5::VertexBuffer* VertexBuffer5Impl::current = nullptr;

VertexBuffer5Impl::VertexBuffer5Impl(int count) : myCount(count) {}

Graphics5::VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, bool gpuMemory, int instanceDataStepRate) : VertexBuffer5Impl(count) {
	this->gpuMemory = gpuMemory;
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		VertexElement element = structure.elements[i];
		switch (element.data) {
		case Graphics4::ColorVertexData:
			myStride += 1 * 4;
			break;
		case Graphics4::Float1VertexData:
			myStride += 1 * 4;
			break;
		case Graphics4::Float2VertexData:
			myStride += 2 * 4;
			break;
		case Graphics4::Float3VertexData:
			myStride += 3 * 4;
			break;
		case Graphics4::Float4VertexData:
			myStride += 4 * 4;
			break;
		case Graphics4::NoVertexData:
			break;
		case Graphics4::Float4x4VertexData:
			assert(false);
			break;
		}
	}

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
	mtlBuffer = [device newBufferWithLength:count * myStride options:options];
}

Graphics5::VertexBuffer::~VertexBuffer() {
	unset();
}

float* Graphics5::VertexBuffer::lock() {
	lastStart = 0;
	lastCount = count();
	id<MTLBuffer> buffer = mtlBuffer;
	float* floats = (float*)[buffer contents];
	return floats;
}

float* Graphics5::VertexBuffer::lock(int start, int count) {
	lastStart = start;
	lastCount = count;
	id<MTLBuffer> buffer = mtlBuffer;
	float* floats = (float*)[buffer contents];
	return &floats[start * myStride / sizeof(float)];
}

void Graphics5::VertexBuffer::unlock() {
#ifndef KORE_IOS
	if (gpuMemory) {
		id<MTLBuffer> buffer = mtlBuffer;
		NSRange range;
		range.location = lastStart * myStride;
		range.length = lastCount * myStride;
		[buffer didModifyRange:range];
	}
#endif
}

int Graphics5::VertexBuffer::_set(int offset_) {
	current = this;
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:mtlBuffer offset:offset_ * myStride atIndex:0];

	return offset_;
}

void VertexBuffer5Impl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int Graphics5::VertexBuffer::count() {
	return myCount;
}

int Graphics5::VertexBuffer::stride() {
	return myStride;
}
