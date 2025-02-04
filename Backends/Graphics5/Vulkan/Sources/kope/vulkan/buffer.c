#include "buffer_functions.h"

#include <kope/graphics5/buffer.h>

void kope_vulkan_buffer_set_name(kope_g5_buffer *buffer, const char *name) {
	const VkDebugMarkerObjectNameInfoEXT name_info = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
	    .pNext = NULL,
	    .objectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
	    .object = (uint64_t)buffer->vulkan.buffer,
	    .pObjectName = name,
	};

	vulkan_DebugMarkerSetObjectNameEXT(buffer->vulkan.device, &name_info);
}

void kope_vulkan_buffer_destroy(kope_g5_buffer *buffer) {
	vkFreeMemory(buffer->vulkan.device, buffer->vulkan.memory, NULL);
	vkDestroyBuffer(buffer->vulkan.device, buffer->vulkan.buffer, NULL);
}

void *kope_vulkan_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	void *data = NULL;
	VkResult result = vkMapMemory(buffer->vulkan.device, buffer->vulkan.memory, 0, buffer->vulkan.size, 0, &data);
	assert(result == VK_SUCCESS);
	return data;
}

void *kope_vulkan_buffer_lock_all(kope_g5_buffer *buffer) {
	void *data = NULL;
	VkResult result = vkMapMemory(buffer->vulkan.device, buffer->vulkan.memory, 0, buffer->vulkan.size, 0, &data);
	assert(result == VK_SUCCESS);
	return data;
}

void *kope_vulkan_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	void *data = NULL;
	VkResult result = vkMapMemory(buffer->vulkan.device, buffer->vulkan.memory, offset, size, 0, &data);
	assert(result == VK_SUCCESS);
	return data;
}

void *kope_vulkan_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	void *data = NULL;
	VkResult result = vkMapMemory(buffer->vulkan.device, buffer->vulkan.memory, offset, size, 0, &data);
	assert(result == VK_SUCCESS);
	return data;
}

void kope_vulkan_buffer_unlock(kope_g5_buffer *buffer) {
	vkUnmapMemory(buffer->vulkan.device, buffer->vulkan.memory);
}
