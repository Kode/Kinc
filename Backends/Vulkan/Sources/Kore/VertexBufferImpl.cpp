#include "pch.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Graphics.h>
#include "ShaderImpl.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

extern VkDevice device;
extern VkCommandBuffer draw_cmd;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

VertexBuffer* VertexBufferImpl::current = nullptr;

namespace {
	const int multiple = 100;
}

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {

}

VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(vertexCount, instanceDataStepRate) {
	index = 0;
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
		case Float4x4VertexData:
			myStride += 4 * 4 * 4;
			break;
		}
	}
	this->structure = structure;
	
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = vertexCount * myStride * multiple;
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

	//createVertexInfo(structure, vertices.info);
}

VertexBuffer::~VertexBuffer() {
	unset();
}

float* VertexBuffer::lock() {
	++index;
	if (index >= multiple) {
		index = 0;
	}
	VkResult err = vkMapMemory(device, vertices.mem, index * myCount * myStride, myCount * myStride, 0, (void**)&data);
	assert(!err);
	return data;
}

void VertexBuffer::unlock() {
	vkUnmapMemory(device, vertices.mem);
}

int VertexBuffer::_set(int offset) {
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();


	VkDeviceSize offsets[1] = { index * myCount * myStride };
	vkCmdBindVertexBuffers(draw_cmd, 0, 1, &vertices.buf, offsets);

	return offsetoffset;
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

int VertexBufferImpl::setVertexAttributes(int offset) {
	
	return 0;
}
