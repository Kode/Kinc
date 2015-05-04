#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

IndexBuffer* IndexBufferImpl::current = nullptr;
const int more = 10;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {
	id <MTLDevice> device = getMetalDevice();
	mtlBuffer = [device newBufferWithLength:sizeof(int) * indexCount * more options:MTLResourceOptionCPUCacheModeDefault];
	index = -1;
}

IndexBuffer::~IndexBuffer() {
	unset();

}

int* IndexBuffer::lock() {
	++index;
	if (index >= more) index = 0;

	id <MTLBuffer> buffer = mtlBuffer;
	int* ints = (int*)[buffer contents];
	return &ints[index * myCount];
	//return (int*)[buffer contents];
}

void IndexBuffer::unlock() {
	
}

void IndexBuffer::set() {
	current = this;
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBufferImpl::offset() {
	return index * myCount * sizeof(int);
}

int IndexBuffer::count() {
	return myCount;
}
