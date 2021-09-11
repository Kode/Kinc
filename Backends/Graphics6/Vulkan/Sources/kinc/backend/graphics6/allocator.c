#include "allocator.h"
#include "vulkan.h"

static VkPhysicalDeviceMemoryProperties memory_properties = {0};

void kinc_vulkan_allocator_init() {
	vkGetPhysicalDeviceMemoryProperties(context.gpu, &memory_properties);
}

void kinc_vulkan_allocator_destroy() {}

static int find_memory_type_index(uint32_t type_bits, VkMemoryPropertyFlags properties) {
	for (uint32_t i = 0; i < 32; i++) {
		if ((type_bits & 1) == 1) {
			if ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		type_bits >>= 1;
	}
	return -1;
}

void kinc_vulkan_alloc(kinc_vulkan_memory_t *memory, const kinc_vulkan_alloc_descriptor_t *desc) {
	int type_index = find_memory_type_index(desc->requirements.memoryTypeBits, desc->flags);
	if (type_index == -1) ERROR("Failed to find memory_type.");

	// TODO : actual allocator

	VkMemoryAllocateInfo allocate_info = {0};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.memoryTypeIndex = type_index;
	allocate_info.allocationSize = desc->requirements.size;
	CHECK(vkAllocateMemory(context.device, &allocate_info, NULL, &memory->memory));
	memory->offset = 0;
	memory->size = desc->requirements.size;
	memory->type = memory_properties.memoryTypes[type_index];
	vkMapMemory(context.device, memory->memory, 0, allocate_info.allocationSize, 0, &memory->ptr);
}
void kinc_vulkan_dealloc(kinc_vulkan_memory_t *memory) {
	vkUnmapMemory(context.device, memory->memory);
	vkFreeMemory(context.device, memory->memory, NULL);
}