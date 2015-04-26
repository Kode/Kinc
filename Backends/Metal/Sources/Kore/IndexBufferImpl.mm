#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

IndexBuffer* IndexBufferImpl::current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {
	id <MTLDevice> device = getMetalDevice();
	mtlBuffer = [device newBufferWithLength:sizeof(int) * indexCount options:MTLResourceOptionCPUCacheModeDefault];
}

IndexBuffer::~IndexBuffer() {
	unset();

}

int* IndexBuffer::lock() {
	id <MTLBuffer> buffer = mtlBuffer;
	return (int*)[buffer contents];
}

void IndexBuffer::unlock() {

}

void IndexBuffer::set() {
	current = this;
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBuffer::count() {
	return myCount;
}
