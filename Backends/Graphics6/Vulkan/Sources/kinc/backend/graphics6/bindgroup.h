#pragma once

#include <vulkan/vulkan_core.h>

typedef struct kinc_g6_bind_group_layout_impl {
    VkDescriptorSetLayout layout;
    uint32_t count;
    struct {
        VkDescriptorType type;
        uint32_t binding;
    } *types;
} kinc_g6_bind_group_layout_impl_t;

typedef struct kinc_g6_bind_group_impl {
    VkDescriptorSet set;
} kinc_g6_bind_group_impl_t;