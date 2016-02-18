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

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

#define VERTEX_BUFFER_BIND_ID 0

VertexBuffer* VertexBufferImpl::current = nullptr;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {

}

VertexBuffer::VertexBuffer(int vertexCount, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(vertexCount, instanceDataStepRate) {
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
	

	data = new float[vertexCount * myStride / 4];

	const float vb[3][5] = {
		/*      position             texcoord */
		{ -1.0f, -1.0f,  0.25f,     0.0f, 0.0f },
		{ 1.0f, -1.0f,  0.25f,     1.0f, 0.0f },
		{ 0.0f,  1.0f,  1.0f,      0.5f, 1.0f },
	};
	
	VkBufferCreateInfo buf_info;
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = sizeof(vb);
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buf_info.flags = 0;
	
	VkMemoryAllocateInfo mem_alloc;
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs;
	VkResult err;
	bool pass;
	void *data;

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

	err = vkMapMemory(device, vertices.mem, 0, mem_alloc.allocationSize, 0, &data);
	assert(!err);

	memcpy(data, vb, sizeof(vb));

	vkUnmapMemory(device, vertices.mem);

	err = vkBindBufferMemory(device, vertices.buf, vertices.mem, 0);
	assert(!err);

	vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertices.vi.pNext = NULL;
	vertices.vi.vertexBindingDescriptionCount = 1;
	vertices.vi.pVertexBindingDescriptions = vertices.vi_bindings;
	vertices.vi.vertexAttributeDescriptionCount = 2;
	vertices.vi.pVertexAttributeDescriptions = vertices.vi_attrs;

	vertices.vi_bindings[0].binding = VERTEX_BUFFER_BIND_ID;
	vertices.vi_bindings[0].stride = sizeof(vb[0]);
	vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	vertices.vi_attrs[0].binding = VERTEX_BUFFER_BIND_ID;
	vertices.vi_attrs[0].location = 0;
	vertices.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertices.vi_attrs[0].offset = 0;

	vertices.vi_attrs[1].binding = VERTEX_BUFFER_BIND_ID;
	vertices.vi_attrs[1].location = 1;
	vertices.vi_attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertices.vi_attrs[1].offset = sizeof(float) * 3;
}

VertexBuffer::~VertexBuffer() {
	unset();
	delete[] data;
}

float* VertexBuffer::lock() {
	return data;
}

void VertexBuffer::unlock() {
	
}

int VertexBuffer::_set(int offset) {
	int offsetoffset = setVertexAttributes(offset);
	if (IndexBuffer::current != nullptr) IndexBuffer::current->_set();
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
