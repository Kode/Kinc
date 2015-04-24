#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include "ShaderImpl.h"
#import <Metal/Metal.h>

using namespace Kore;

void* getMetalDevice();

VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count) : myCount(count) {

}

VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure) : VertexBufferImpl(vertexCount) {
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		VertexElement element = structure.elements[i];
		switch (element.data) {
		case ColorVertexData:
			myStride += 1 * 4;
			break;
		case Float1VertexData:
			myStride += 1 * 4;
			break;
		case Float2VertexData:
			myStride += 2 * 4;
			break;
		case Float3VertexData:
			myStride += 3 * 4;
			break;
		case Float4VertexData:
			myStride += 4 * 4;
			break;
		case NoVertexData:
			break;
		}
	}
	
	id <MTLDevice> device = (__bridge_transfer id <MTLDevice>)getMetalDevice();
	mtlBuffer = (__bridge_retained void*)[device newBufferWithLength:sizeof(float) * vertexCount * myStride / 4 options:MTLResourceOptionCPUCacheModeDefault];
}

VertexBuffer::~VertexBuffer() {
	unset();
	
}

float* VertexBuffer::lock() {
	id <MTLBuffer> buffer = (__bridge_transfer id <MTLBuffer>)mtlBuffer;
	return (float*)[buffer contents];
}

void VertexBuffer::unlock() {

}

void VertexBuffer::set() {
	current = this;
	if (IndexBuffer::current != nullptr) IndexBuffer::current->set();
}

void VertexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}
