#pragma once

#include "MiniVulkan.h"

struct texture_object {
	VkSampler sampler;

	VkImage image;
	VkImageLayout imageLayout;

	VkDeviceMemory mem;
	VkImageView view;
	int32_t tex_width, tex_height;
};

typedef struct {
	uint8_t pixfmt;

	struct texture_object texture;
	VkDeviceSize deviceSize;

	uint8_t *conversionBuffer;
	int stride;
} Texture5Impl;
