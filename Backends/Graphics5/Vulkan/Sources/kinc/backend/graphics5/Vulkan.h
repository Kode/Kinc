#pragma once

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/image.h>
#include <kinc/math/matrix.h>

namespace Kore {
	namespace Vulkan {
		struct SwapchainBuffers {
			VkImage image;
			VkCommandBuffer cmd;
			VkImageView view;
		};

		extern SwapchainBuffers *buffers;

		struct DepthBuffer {
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		};

		extern DepthBuffer depth;

		// buffer hack
		extern VkBuffer *vertexUniformBuffer;
		extern VkBuffer *fragmentUniformBuffer;
	}
}
