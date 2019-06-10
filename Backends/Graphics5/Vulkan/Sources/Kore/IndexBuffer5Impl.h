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
	//void unset();

	int *data;
	int myCount;
	unsigned bufferId;

	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;

	//static Graphics5::IndexBuffer* current;
} IndexBuffer5Impl;
