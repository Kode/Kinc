#pragma once

#include <vulkan/vulkan.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef RegisterClass
#undef RegisterClass
#endif

typedef struct {
	VkBuffer buf;
	VkDescriptorBufferInfo buffer_info;
	VkMemoryAllocateInfo mem_alloc;
	VkDeviceMemory mem;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;
