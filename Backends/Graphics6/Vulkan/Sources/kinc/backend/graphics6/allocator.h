#pragma once

#include <vulkan/vulkan_core.h>

void kinc_vulkan_allocator_init();
void kinc_vulkan_allocator_destroy();

typedef struct kinc_vulkan_memory {
	VkDeviceMemory memory;
	VkDeviceSize size;
	VkDeviceSize offset;
	VkMemoryType type;
	void *ptr;
} kinc_vulkan_memory_t;

typedef struct kinc_vulkan_alloc_descriptor {
    VkMemoryRequirements requirements;
    VkMemoryPropertyFlags flags;
} kinc_vulkan_alloc_descriptor_t;

void kinc_vulkan_alloc(kinc_vulkan_memory_t *memory, const kinc_vulkan_alloc_descriptor_t *descriptor);
void kinc_vulkan_dealloc(kinc_vulkan_memory_t *memory);