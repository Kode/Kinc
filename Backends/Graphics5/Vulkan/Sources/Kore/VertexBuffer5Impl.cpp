#include "pch.h"

#include "Shader5Impl.h"
#include "VertexBuffer5Impl.h"

#include <Kore/Graphics5/Graphics.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

using namespace Kore;

extern VkDevice device;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

Graphics5::VertexBuffer* VertexBuffer5Impl::current = nullptr;

VertexBuffer5Impl::VertexBuffer5Impl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {}

Graphics5::VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure, bool gpuMemory, int instanceDataStepRate) : VertexBuffer5Impl(vertexCount, instanceDataStepRate) {
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
		case Graphics4::Float4x4VertexData:
			myStride += 4 * 4 * 4;
			break;
		}
	}
	this->structure = structure;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = vertexCount * myStride;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.flags = 0;

	memset(&mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs = {};
	VkResult err;
	bool pass;

	memset(&vertices, 0, sizeof(vertices));

	err = vkCreateBuffer(device, &buf_info, NULL, &vertices.buf);
	assert(!err);

	vkGetBufferMemoryRequirements(device, vertices.buf, &mem_reqs);
	assert(!err);

	mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex);
	assert(pass);

	err = vkAllocateMemory(device, &mem_alloc, NULL, &vertices.mem);
	assert(!err);

	err = vkBindBufferMemory(device, vertices.buf, vertices.mem, 0);
	assert(!err);

	// createVertexInfo(structure, vertices.info);
}

Graphics5::VertexBuffer::~VertexBuffer() {
	unset();
}

float* Graphics5::VertexBuffer::lock() {
	return lock(0, myCount);
}

float* Graphics5::VertexBuffer::lock(int start, int count) {
	VkResult err = vkMapMemory(device, vertices.mem, start * myStride, count * myStride, 0, (void**)&data);
	assert(!err);
	return data;
}

void Graphics5::VertexBuffer::unlock() {
	vkUnmapMemory(device, vertices.mem);
}

int Graphics5::VertexBuffer::_set(int offset) {
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();
		
	return offsetoffset;
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

int VertexBuffer5Impl::setVertexAttributes(int offset) {

	return 0;
}
