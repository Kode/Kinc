#pragma once

#include "MiniVulkan.h"

typedef struct {
	int *data;
	int myCount;
	unsigned bufferId;

	VkBuffer buf;
	VkDeviceMemory mem;
	VkMemoryAllocateInfo mem_alloc;
} IndexBuffer5Impl;
