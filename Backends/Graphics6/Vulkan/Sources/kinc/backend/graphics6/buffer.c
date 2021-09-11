#include "vulkan.h"
#include <kinc/graphics6/buffer.h>

void kinc_g6_buffer_init(kinc_g6_buffer_t *buffer, const kinc_g6_buffer_descriptor_t *descriptor) {
	VkBufferCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.size = descriptor->size;
	create_info.usage = 0;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_COPY_SRC) create_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_COPY_DST) create_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_VERTEX) create_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_INDEX) create_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_UNIFORM) create_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (descriptor->usage & KINC_G6_BUFFER_USAGE_STORAGE) create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0;
	create_info.pQueueFamilyIndices = NULL;

	CHECK(vkCreateBuffer(context.device, &create_info, NULL, &buffer->impl.buffer));

	kinc_vulkan_alloc_descriptor_t desc = {0};
	vkGetBufferMemoryRequirements(context.device, buffer->impl.buffer, &desc.requirements);
	desc.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	if(descriptor->gpuMemory) desc.flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	kinc_vulkan_alloc(&buffer->impl.memory, &desc);
	CHECK(vkBindBufferMemory(context.device, buffer->impl.buffer, buffer->impl.memory.memory, buffer->impl.memory.offset));
}

void kinc_g6_buffer_destroy(kinc_g6_buffer_t *buffer) {
	vkDestroyBuffer(context.device, buffer->impl.buffer, NULL);
}

void *kinc_g6_buffer_map(kinc_g6_buffer_t *buffer, int start, int range) {
	return (void*)((char*)buffer->impl.memory.ptr + start);
}

void kinc_g6_buffer_unmap(kinc_g6_buffer_t *buffer) {}