#pragma once

#include <kinc/image.h>
#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/math/matrix.h>

struct PipelineState5Impl_s;

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
		void createDescriptorSet(struct PipelineState5Impl_s *pipeline, VkDescriptorSet &desc_set);
	}
}
