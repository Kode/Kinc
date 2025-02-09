#ifndef KOPE_VULKAN_DEVICE_STRUCTS_HEADER
#define KOPE_VULKAN_DEVICE_STRUCTS_HEADER

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/texture.h>
#include <kope/util/indexallocator.h>
#include <kope/util/offalloc/offalloc.h>

#include "vulkanmini.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_vulkan_device {
	VkDevice device;
	VkCommandPool command_pool;
	VkQueue queue;
	VkPhysicalDeviceMemoryProperties device_memory_properties;
} kope_vulkan_device;

typedef struct kope_vulkan_query_set {
	int nothing;
} kope_vulkan_query_set;

typedef struct kope_vulkan_raytracing_volume {
	int nothing;
} kope_vulkan_raytracing_volume;

typedef struct kope_vulkan_raytracing_hierarchy {
	int nothing;
} kope_vulkan_raytracing_hierarchy;

#ifdef __cplusplus
}
#endif

#endif
