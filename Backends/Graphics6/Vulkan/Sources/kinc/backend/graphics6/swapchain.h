#pragma once

#include <kinc/graphics6/texture.h>
#include <vulkan/vulkan_core.h>

typedef struct kinc_g6_swapchain_impl {
	int swapchain_image_count;
	VkFormat format;
	VkColorSpaceKHR color_space;
	VkExtent2D extent;
	VkSwapchainKHR swapchain;
	VkSurfaceKHR surface;

	VkImage *images;
	// VkImageView *views;

	VkSemaphore acquireSem;

	kinc_g6_texture_t swap_texture;
	uint32_t current_index;
} kinc_g6_swapchain_impl_t;