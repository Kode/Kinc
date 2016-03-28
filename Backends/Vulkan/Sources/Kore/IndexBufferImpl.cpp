#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <string.h>

using namespace Kore;

extern VkDevice device;
extern VkCommandBuffer draw_cmd;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

IndexBuffer* IndexBufferImpl::current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {

}

IndexBuffer::IndexBuffer(int indexCount) : IndexBufferImpl(indexCount) {
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = indexCount * sizeof(int);
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buf_info.flags = 0;

	memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	memset(&buf, 0, sizeof(buf));
	memset(&mem, 0, sizeof(mem));

	VkResult err = vkCreateBuffer(device, &buf_info, NULL, &buf);
	assert(!err);

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(device, buf, &mem_reqs);
	assert(!err);

	mem_alloc.allocationSize = mem_reqs.size;
	bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
	assert(pass);

	err = vkAllocateMemory(device, &mem_alloc, NULL, &mem);
	assert(!err);

	err = vkBindBufferMemory(device, buf, mem, 0);
	assert(!err);
}

IndexBuffer::~IndexBuffer() {
	unset();
	delete[] data;
}

int* IndexBuffer::lock() {
	VkResult err = vkMapMemory(device, mem, 0, mem_alloc.allocationSize, 0, (void**)&data);
	assert(!err);
	return data;
}

void IndexBuffer::unlock() {
	vkUnmapMemory(device, mem);
}

void IndexBuffer::_set() {
	current = this;

	vkCmdBindIndexBuffer(draw_cmd, buf, 0, VK_INDEX_TYPE_UINT32);
}

void IndexBufferImpl::unset() {
	if ((void*)current == (void*)this) current = nullptr;
}

int IndexBuffer::count() {
	return myCount;
}
