#include "pch.h"

#include "RenderTarget5Impl.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/log.h>

#include <vulkan/vulkan.h>

#include <assert.h>

extern VkDevice device;
extern VkRenderPass render_pass;
extern uint32_t swapchainImageCount;
extern VkPhysicalDevice gpu;
extern kinc_g5_texture_t *vulkanTextures[8];
extern kinc_g5_render_target_t *vulkanRenderTargets[8];

bool memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex);

extern VkCommandPool cmd_pool;
extern VkQueue queue;
static VkCommandBuffer setup_cmd;

namespace {
	void demo_flush_init_cmd() {
		VkResult err;

		if (setup_cmd == VK_NULL_HANDLE) return;

		err = vkEndCommandBuffer(setup_cmd);
		assert(!err);

		const VkCommandBuffer cmd_bufs[] = {setup_cmd};
		VkFence nullFence = {VK_NULL_HANDLE};
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = NULL;
		submit_info.pWaitDstStageMask = NULL;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = cmd_bufs;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
		assert(!err);

		err = vkQueueWaitIdle(queue);
		assert(!err);

		vkFreeCommandBuffers(device, cmd_pool, 1, cmd_bufs);
		setup_cmd = VK_NULL_HANDLE;
	}

	void demo_setup_init_cmd() {
		if (setup_cmd == VK_NULL_HANDLE) {
			VkCommandBufferAllocateInfo cmd = {};
			cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd.pNext = NULL;
			cmd.commandPool = cmd_pool;
			cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd.commandBufferCount = 1;

			VkResult err = vkAllocateCommandBuffers(device, &cmd, &setup_cmd);
			assert(!err);

			VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
			cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			cmd_buf_hinfo.pNext = NULL;
			cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
			cmd_buf_hinfo.subpass = 0;
			cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
			cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
			cmd_buf_hinfo.queryFlags = 0;
			cmd_buf_hinfo.pipelineStatistics = 0;

			VkCommandBufferBeginInfo cmd_buf_info = {};
			cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmd_buf_info.pNext = NULL;
			cmd_buf_info.flags = 0;
			cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

			err = vkBeginCommandBuffer(setup_cmd, &cmd_buf_info);
			assert(!err);
		}
	}
}

void setImageLayout(VkCommandBuffer _buffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout) {

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	// if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED) imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED) imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED) imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		if (oldImageLayout != VK_IMAGE_LAYOUT_UNDEFINED) imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(_buffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, int depthBufferBits, bool antialiasing,
                                    kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {
	target->width = width;
	target->height = height;
	target->contextId = contextId;
	target->texWidth = width;
	target->texHeight = height;

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	VkResult err = vkCreateSampler(device, &samplerInfo, nullptr, &target->impl.sampler);
	assert(!err);

	if (contextId >= 0) {

		{
		    VkFormatProperties formatProperties;
		    VkResult err;

		    vkGetPhysicalDeviceFormatProperties(gpu, VK_FORMAT_B8G8R8A8_UNORM, &formatProperties);
		    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

		    VkImageCreateInfo imageCreateInfo = {};
		    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		    imageCreateInfo.pNext = NULL;
		    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		    imageCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		    imageCreateInfo.extent = {(uint32_t)width, (uint32_t)height, 1};
		    imageCreateInfo.mipLevels = 1;
		    imageCreateInfo.arrayLayers = 1;
		    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		    imageCreateInfo.flags = 0;
		    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

		    err = vkCreateImage(device, &imageCreateInfo, nullptr, &target->impl.destImage);
		    assert(!err);

		    VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(device, target->impl.destImage, &memoryRequirements);

		    VkMemoryAllocateInfo allocationInfo = {};
		    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		    allocationInfo.pNext = nullptr;
		    allocationInfo.memoryTypeIndex = 0;

		    vkGetImageMemoryRequirements(device, target->impl.destImage, &memoryRequirements);
		    allocationInfo.allocationSize = memoryRequirements.size;
		    bool pass = memory_type_from_properties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocationInfo.memoryTypeIndex);
		    assert(pass);
			err = vkAllocateMemory(device, &allocationInfo, nullptr, &target->impl.destMemory);
		    assert(!err);
			err = vkBindImageMemory(device, target->impl.destImage, target->impl.destMemory, 0);
		    assert(!err);

		    demo_setup_init_cmd();
		    setImageLayout(setup_cmd, target->impl.destImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		    demo_flush_init_cmd();

		    VkImageViewCreateInfo view = {};
		    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		    view.pNext = nullptr;
		    view.image = VK_NULL_HANDLE;
		    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		    view.format = VK_FORMAT_B8G8R8A8_UNORM;
		    view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
		    view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			view.image = target->impl.destImage;
			err = vkCreateImageView(device, &view, nullptr, &target->impl.destView);
		    assert(!err);
		}

		{
		    VkFormatProperties formatProperties;
		    VkResult err;

		    vkGetPhysicalDeviceFormatProperties(gpu, VK_FORMAT_B8G8R8A8_UNORM, &formatProperties);
		    assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);

		    VkImageCreateInfo image = {};
		    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		    image.pNext = nullptr;
		    image.imageType = VK_IMAGE_TYPE_2D;
		    image.format = VK_FORMAT_B8G8R8A8_UNORM;
		    image.extent.width = width;
		    image.extent.height = height;
		    image.extent.depth = 1;
		    image.mipLevels = 1;
		    image.arrayLayers = 1;
		    image.samples = VK_SAMPLE_COUNT_1_BIT;
		    image.tiling = VK_IMAGE_TILING_OPTIMAL;
		    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		    image.flags = 0;

		    VkImageViewCreateInfo colorImageView = {};
		    colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		    colorImageView.pNext = nullptr;
		    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		    colorImageView.format = VK_FORMAT_B8G8R8A8_UNORM;
		    colorImageView.flags = 0;
		    colorImageView.subresourceRange = {};
		    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		    colorImageView.subresourceRange.baseMipLevel = 0;
		    colorImageView.subresourceRange.levelCount = 1;
		    colorImageView.subresourceRange.baseArrayLayer = 0;
		    colorImageView.subresourceRange.layerCount = 1;

		    err = vkCreateImage(device, &image, nullptr, &target->impl.sourceImage);
		    assert(!err);

		    VkMemoryRequirements memoryRequirements;
			vkGetImageMemoryRequirements(device, target->impl.sourceImage, &memoryRequirements);

		    VkMemoryAllocateInfo allocationInfo = {};
		    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		    allocationInfo.pNext = nullptr;
		    allocationInfo.memoryTypeIndex = 0;

		    vkGetImageMemoryRequirements(device, target->impl.sourceImage, &memoryRequirements);
		    allocationInfo.allocationSize = memoryRequirements.size;
		    bool pass = memory_type_from_properties(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocationInfo.memoryTypeIndex);
		    assert(pass);

		    err = vkAllocateMemory(device, &allocationInfo, nullptr, &target->impl.sourceMemory);
		    assert(!err);

		    err = vkBindImageMemory(device, target->impl.sourceImage, target->impl.sourceMemory, 0);
		    assert(!err);

		    demo_setup_init_cmd();
			setImageLayout(setup_cmd, target->impl.sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			demo_flush_init_cmd();

		    colorImageView.image = target->impl.sourceImage;
			err = vkCreateImageView(device, &colorImageView, nullptr, &target->impl.sourceView);
		    assert(!err);
		}

		{
			const VkFormat depth_format = VK_FORMAT_D16_UNORM;
			VkImageCreateInfo image = {};
			image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image.pNext = NULL;
			image.imageType = VK_IMAGE_TYPE_2D;
			image.format = depth_format;
			image.extent.width = width;
			image.extent.height = height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = 1;
			image.samples = VK_SAMPLE_COUNT_1_BIT;
			image.tiling = VK_IMAGE_TILING_OPTIMAL;
			image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image.flags = 0;

			/* create image */
			err = vkCreateImage(device, &image, NULL, &target->impl.depthImage);
			assert(!err);

			VkMemoryAllocateInfo mem_alloc = {};
			mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			mem_alloc.pNext = NULL;
			mem_alloc.allocationSize = 0;
			mem_alloc.memoryTypeIndex = 0;

			VkImageViewCreateInfo view = {};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.pNext = NULL;
			view.image = target->impl.depthImage;
			view.format = depth_format;
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view.subresourceRange.baseMipLevel = 0;
			view.subresourceRange.levelCount = 1;
			view.subresourceRange.baseArrayLayer = 0;
			view.subresourceRange.layerCount = 1;
			view.flags = 0;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;

			VkMemoryRequirements mem_reqs = {};
			bool pass;

			/* get memory requirements for this object */
			vkGetImageMemoryRequirements(device, target->impl.depthImage, &mem_reqs);

			/* select memory size and type */
			mem_alloc.allocationSize = mem_reqs.size;
			pass = memory_type_from_properties(mem_reqs.memoryTypeBits, 0, /* No requirements */ &mem_alloc.memoryTypeIndex);
			assert(pass);

			/* allocate memory */
			err = vkAllocateMemory(device, &mem_alloc, NULL, &target->impl.depthMemory);
			assert(!err);

			/* bind memory */
			err = vkBindImageMemory(device, target->impl.depthImage, target->impl.depthMemory, 0);
			assert(!err);

			demo_setup_init_cmd();
			setImageLayout(setup_cmd, target->impl.depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			demo_flush_init_cmd();

			/* create image view */
			err = vkCreateImageView(device, &view, NULL, &target->impl.depthView);
			assert(!err);
		}

		{
			VkAttachmentDescription attachments[2];
			attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments[0].flags = 0;

			attachments[1].format = VK_FORMAT_D16_UNORM;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments[1].flags = 0;

			VkAttachmentReference color_reference = {};
			color_reference.attachment = 0;
			color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depth_reference = {};
			depth_reference.attachment = 1;
			depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.flags = 0;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_reference;
			subpass.pResolveAttachments = nullptr;
			subpass.pDepthStencilAttachment = &depth_reference;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;

			VkRenderPassCreateInfo rp_info = {};
			rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			rp_info.pNext = nullptr;
			rp_info.attachmentCount = 2;
			rp_info.pAttachments = attachments;
			rp_info.subpassCount = 1;
			rp_info.pSubpasses = &subpass;
			rp_info.dependencyCount = 0;
			rp_info.pDependencies = nullptr;

			err = vkCreateRenderPass(device, &rp_info, NULL, &target->impl.renderPass);
			assert(!err);
		}

		VkImageView attachments[2];
		attachments[0] = target->impl.sourceView;
		attachments[1] = target->impl.depthView;

		VkFramebufferCreateInfo fbufCreateInfo = {};
		fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbufCreateInfo.pNext = nullptr;
		fbufCreateInfo.renderPass = target->impl.renderPass;
		fbufCreateInfo.attachmentCount = 2;
		fbufCreateInfo.pAttachments = attachments;
		fbufCreateInfo.width = width;
		fbufCreateInfo.height = height;
		fbufCreateInfo.layers = 1;

		err = vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &target->impl.framebuffer);
		assert(!err);
	}
}

void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *target, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target) {}

void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit) {
	vulkanRenderTargets[unit.impl.binding - 2] = target;
	vulkanTextures[unit.impl.binding - 2] = nullptr;
}

void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit) {}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source) {}
