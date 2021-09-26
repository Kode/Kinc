#include <kinc/graphics5/indexbuffer.h>

extern VkDevice device;

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

kinc_g5_index_buffer_t *currentIndexBuffer = NULL;

static void unset(kinc_g5_index_buffer_t *buffer) {
	if (currentIndexBuffer == buffer) {
		currentIndexBuffer = NULL;
	}
}

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.myCount = indexCount;
	VkBufferCreateInfo buf_info = {};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = NULL;
	buf_info.size = indexCount * sizeof(int);
	buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
#ifdef KORE_VKRT
	buf_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#endif
	buf_info.flags = 0;

	memset(&buffer->impl.mem_alloc, 0, sizeof(VkMemoryAllocateInfo));
	buffer->impl.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	buffer->impl.mem_alloc.pNext = NULL;
	buffer->impl.mem_alloc.allocationSize = 0;
	buffer->impl.mem_alloc.memoryTypeIndex = 0;

	memset(&buffer->impl.buf, 0, sizeof(buffer->impl.buf));
	memset(&buffer->impl.mem, 0, sizeof(buffer->impl.mem));

	VkResult err = vkCreateBuffer(device, &buf_info, NULL, &buffer->impl.buf);
	assert(!err);

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(device, buffer->impl.buf, &mem_reqs);
	assert(!err);

	buffer->impl.mem_alloc.allocationSize = mem_reqs.size;
	bool pass = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer->impl.mem_alloc.memoryTypeIndex);
	assert(pass);

#ifdef KORE_VKRT
	VkMemoryAllocateFlagsInfo memory_allocate_flags_info = {};
	memory_allocate_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memory_allocate_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	buffer->impl.mem_alloc.pNext = &memory_allocate_flags_info;
#endif

	err = vkAllocateMemory(device, &buffer->impl.mem_alloc, NULL, &buffer->impl.mem);
	assert(!err);

	err = vkBindBufferMemory(device, buffer->impl.buf, buffer->impl.mem, 0);
	assert(!err);
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	unset(buffer);
	delete[] buffer->impl.data;
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	VkResult err = vkMapMemory(device, buffer->impl.mem, 0, buffer->impl.mem_alloc.allocationSize, 0, (void **)&buffer->impl.data);
	assert(!err);
	return buffer->impl.data;
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	vkUnmapMemory(device, buffer->impl.mem);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {
	currentIndexBuffer = buffer;
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.myCount;
}
