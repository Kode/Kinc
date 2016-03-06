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

namespace Kore {
	class RenderTargetImpl {
	public:
		VkImage destImage;
		VkDeviceMemory destMemory;
		VkImageView destView;

		VkImage sourceImage;
		VkDeviceMemory sourceMemory;
		VkImageView sourceView;

		VkFramebuffer framebuffer;
		VkDescriptorSet desc_set;
		VkSampler sampler;
		VkRenderPass renderPass;
	};
}
