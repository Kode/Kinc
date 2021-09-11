#pragma once

#include <vulkan/vulkan_core.h>
#include "allocator.h"

typedef struct kinc_g6_buffer_impl {
    VkBuffer buffer;
    kinc_vulkan_memory_t memory;
} kinc_g6_buffer_impl_t;