#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include <vulkan/vulkan.h>

using namespace Kore;

IndexBuffer* IndexBufferImpl::current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {

	data = new int[indexCount];
}

IndexBuffer::~IndexBuffer() {
	unset();
	delete[] data;
}

int* IndexBuffer::lock() {
	return data;
}

void IndexBuffer::unlock() {

}

void IndexBuffer::_set() {
	current = this;

	//vkCmdBindIndexBuffer(draw_cmd, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBuffer::count() {
	return myCount;
}
