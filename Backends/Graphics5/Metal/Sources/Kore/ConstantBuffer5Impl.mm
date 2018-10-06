#include "pch.h"

#include <Kore/Graphics5/ConstantBuffer.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

ConstantBuffer5Impl::ConstantBuffer5Impl() : _buffer(0) {
	
}

ConstantBuffer5Impl::~ConstantBuffer5Impl() {
	_buffer = 0;
}

Graphics5::ConstantBuffer::ConstantBuffer(int size) {
	mySize = size;
	data = nullptr;
	_buffer = [getMetalDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
}

Graphics5::ConstantBuffer::~ConstantBuffer() {}

void Graphics5::ConstantBuffer::lock() {
	lock(0, size());
}

void Graphics5::ConstantBuffer::lock(int start, int count) {
	lastStart = start;
	lastCount = count;
	u8* data = (u8*)[_buffer contents];
	this->data = &data[start];
}

void Graphics5::ConstantBuffer::unlock() {

	data = nullptr;
}

int Graphics5::ConstantBuffer::size() {
	return mySize;
}
