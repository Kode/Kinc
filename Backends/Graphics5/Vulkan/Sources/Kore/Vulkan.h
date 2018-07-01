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
	}
}
