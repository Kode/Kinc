#pragma once

#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics5/Graphics.h>
#include <Kore/Math/Matrix.h>

namespace Kore {
	namespace Vulkan {
		struct SwapchainBuffers {
			VkImage image;
			VkCommandBuffer cmd;
			VkImageView view;
		};

		extern SwapchainBuffers* buffers;

		struct DepthBuffer {
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		};

		extern DepthBuffer depth;

		// buffer hack
		extern VkBuffer* vertexUniformBuffer;
		extern VkBuffer* fragmentUniformBuffer;

		void demo_set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout);
		void demo_flush_init_cmd();
		void createDescriptorSet(PipelineState5Impl* pipeline, Graphics5::Texture* texture, Graphics5::RenderTarget* renderTarget, VkDescriptorSet& desc_set);
	}
}
