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
		}
	}

	id<MTLDevice> device = getMetalDevice();
	mtlBuffer = [device newBufferWithLength:count * myStride options:MTLResourceOptionCPUCacheModeDefault];
}

Graphics5::VertexBuffer::~VertexBuffer() {
	unset();
}

float* Graphics5::VertexBuffer::lock() {
	id<MTLBuffer> buffer = mtlBuffer;
	float* floats = (float*)[buffer contents];
	return floats;
	//return &floats[index * myStride * myCount / sizeof(float)];
}

float* Graphics5::VertexBuffer::lock(int start, int count) {
	id<MTLBuffer> buffer = mtlBuffer;
	float* floats = (float*)[buffer contents];
	return &floats[start * myStride];
}

void Graphics5::VertexBuffer::unlock() {}

int Graphics5::VertexBuffer::_set(int offset_) {
	current = this;
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:mtlBuffer offset:0 atIndex:0];

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
