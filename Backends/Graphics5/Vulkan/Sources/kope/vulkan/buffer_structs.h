#ifndef KOPE_VULKAN_BUFFER_STRUCTS_HEADER
#define KOPE_VULKAN_BUFFER_STRUCTS_HEADER

#include "vulkanmini.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_g5_device;

typedef struct kope_vulkan_buffer {
	VkDevice device;
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint64_t size;
} kope_vulkan_buffer;

#ifdef __cplusplus
}
#endif

#endif
