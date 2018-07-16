#include "pch.h"

#include "Vulkan.h"

#include <Kore/Graphics5/ConstantBuffer.h>

#include <assert.h>
#include <memory.h>

using namespace Kore;

extern VkDevice device;
bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

VkBuffer* Kore::Vulkan::vertexUniformBuffer = nullptr;
VkBuffer* Kore::Vulkan::fragmentUniformBuffer = nullptr;

namespace {
	void createUniformBuffer(VkBuffer& buf, VkMemoryAllocateInfo& mem_alloc, VkDeviceMemory& mem, VkDescriptorBufferInfo& buffer_info, int size) {
		VkBufferCreateInfo buf_info;
		memset(&buf_info, 0, sizeof(buf_info));
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buf_info.size = sizeof(float) * size;
		VkResult err = vkCreateBuffer(device, &buf_info, NULL, &buf);
		assert(!err);

		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(device, buf, &mem_reqs);

		mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_alloc.pNext = NULL;
		mem_alloc.allocationSize = mem_reqs.size;
		mem_alloc.memoryTypeIndex = 0;

		bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
		assert(pass);

		err = vkAllocateMemory(device, &mem_alloc, NULL, &mem);
		assert(!err);

		err = vkBindBufferMemory(device, buf, mem, 0);
		assert(!err);

		buffer_info.buffer = buf;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(float) * size;
	}
}

Graphics5::ConstantBuffer::ConstantBuffer(int size) {
	mySize = size;
	data = nullptr;

	createUniformBuffer(buf, mem_alloc, mem, buffer_info, size);

	// buffer hack
	if (Vulkan::vertexUniformBuffer == nullptr) {
		Vulkan::vertexUniformBuffer = &buf;
	}
	else if (Vulkan::fragmentUniformBuffer == nullptr) {
		Vulkan::fragmentUniformBuffer = &buf;
	}

	void* p;
	VkResult err = vkMapMemory(device, mem, 0, mem_alloc.allocationSize, 0, (void**)&p);
	assert(!err);
	memset(p, 0, mem_alloc.allocationSize);
	vkUnmapMemory(device, mem);
}

Graphics5::ConstantBuffer::~ConstantBuffer() {}

void Graphics5::ConstantBuffer::lock() {
	lock(0, size());
}

void Graphics5::ConstantBuffer::lock(int start, int count) {
	VkResult err = vkMapMemory(device, mem, start, count, 0, (void**)&data);
	assert(!err);
}

void Graphics5::ConstantBuffer::unlock() {
	vkUnmapMemory(device, mem);
	data = nullptr;
}

int Graphics5::ConstantBuffer::size() {
	return mySize;
}
