#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include <vulkan/vulkan.h>
#include <assert.h>

using namespace Kore;

extern VkDevice device;
extern VkRenderPass render_pass;
extern VkCommandBuffer draw_cmd;
extern uint32_t swapchainImageCount;

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

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits) : width(width), height(height) {
	VkImageView attachments[2];
	attachments[1] = depth.view;

	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;
	fb_info.renderPass = render_pass;
	fb_info.attachmentCount = 2;
	fb_info.pAttachments = attachments;
	fb_info.width = width;
	fb_info.height = height;
	fb_info.layers = 1;

	framebuffers = (VkFramebuffer*)malloc(swapchainImageCount * sizeof(VkFramebuffer));
	assert(framebuffers);

	for (uint32_t i = 0; i < swapchainImageCount; ++i) {
		attachments[0] = buffers[i].view;
		VkResult res = vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffers[i]);
		assert(res == VK_SUCCESS);
	}
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	
}
