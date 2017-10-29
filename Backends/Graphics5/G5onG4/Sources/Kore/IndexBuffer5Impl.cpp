#include "pch.h"

#include "IndexBuffer5Impl.h"

#include <Kore/Graphics5/Graphics.h>

#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

IndexBuffer5Impl* IndexBuffer5Impl::_current = nullptr;

IndexBuffer5Impl::IndexBuffer5Impl(int count, bool gpuMemory) : myCount(count) {
	buffer = new Graphics4::IndexBuffer(count);
}

Graphics5::IndexBuffer::IndexBuffer(int count, bool gpuMemory) : IndexBuffer5Impl(count, gpuMemory) {

}

Graphics5::IndexBuffer::~IndexBuffer() {
	delete buffer;
}

int* Graphics5::IndexBuffer::lock() {
	return buffer->lock();
}

void Graphics5::IndexBuffer::unlock() {
	buffer->unlock();
}

void Graphics5::IndexBuffer::_set() {
	_current = this;
}

int Graphics5::IndexBuffer::count() {
	return myCount;
}
