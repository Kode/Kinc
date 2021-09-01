#include "shader.h"
#include "vertexbuffer.h"

#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

extern VkDevice device;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

kinc_g5_vertex_buffer_t *currentVertexBuffer = NULL;
extern kinc_g5_index_buffer_t *currentIndexBuffer;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int vertexCount, kinc_g5_vertex_structure_t *structure, bool gpuMemory,
                                int instanceDataStepRate) {
	buffer->impl.myCount = vertexCount;
	instanceDataStepRate = instanceDataStepRate;
	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_COLOR:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT1:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			buffer->impl.myStride += 2 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			buffer->impl.myStride += 3 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			buffer->impl.myStride += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			buffer->impl.myStride += 4 * 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			buffer->impl.myStride += 2 * 2;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			buffer->impl.myStride += 4 * 2;
			break;
		}
	}
	buffer->impl.structure = *structure;

	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = vertexCount * buffer->impl.myStride;
	buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
#ifdef KORE_VKRT
	buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#endif
	buf_info.flags = 0;

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements mem_reqs = {};
	VkResult err;
	bool pass;

	memset(&buffer->impl.vertices, 0, sizeof(buffer->impl.vertices));

	err = vkCreateBuffer(device, &buf_info, NULL, &buffer->impl.vertices.buf);
	assert(!err);

	vkGetBufferMemoryRequirements(device, buffer->impl.vertices.buf, &mem_reqs);
	assert(!err);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);
	assert(pass);

#ifdef KORE_VKRT
	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {};
	memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
#endif

	err = vkAllocateMemory(device, &buffer->impl.mem_alloc, NULL, &buffer->impl.vertices.mem);
	assert(!err);

	err = vkBindBufferMemory(device, buffer->impl.vertices.buf, buffer->impl.vertices.mem, 0);
	assert(!err);
}

static void unset(kinc_g5_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer) {
		currentVertexBuffer = NULL;
	}
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	unset(buffer);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_lock(buffer, 0, buffer->impl.myCount);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	VkResult err = vkMapMemory(device, buffer->impl.vertices.mem, start * buffer->impl.myStride, count * buffer->impl.myStride, 0, (void **)&buffer->impl.data);
	assert(!err);
	return buffer->impl.data;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	vkUnmapMemory(device, buffer->impl.vertices.mem);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	vkUnmapMemory(device, buffer->impl.vertices.mem);
}

static int setVertexAttributes(int offset) {
	return 0;
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer, int offset) {
	int offsetoffset = setVertexAttributes(offset);
	return offsetoffset;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}
