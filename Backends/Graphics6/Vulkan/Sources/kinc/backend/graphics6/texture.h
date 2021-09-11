#pragma once

#include <vulkan/vulkan_core.h>

struct kinc_g6_texture_descriptor;

typedef struct kinc_g6_texture_impl {
	VkImage image;
	VkExtent3D extent;
	VkFormat format;
} kinc_g6_texture_impl_t;

typedef struct kinc_g6_texture_view_impl {
	VkImageView view;
	VkExtent3D extent;
	VkFormat format;
} kinc_g6_texture_view_impl_t;
