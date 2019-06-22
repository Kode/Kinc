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

struct texture_object {
	VkSampler sampler;

	VkImage image;
	VkImageLayout imageLayout;

	VkDeviceMemory mem;
	VkImageView view;
	int32_t tex_width, tex_height;
};

typedef struct {
	int binding;
} TextureUnit5Impl;

typedef struct {
	uint8_t pixfmt;

	struct texture_object texture;
	VkDeviceSize deviceSize;

	VkDescriptorSet desc_set;

	uint8_t *conversionBuffer;
} Texture5Impl;
