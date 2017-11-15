#include "pch.h"

#include <Kore/Graphics5/CommandList.h>
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/System.h>

#include <assert.h>

#include <vulkan/vulkan.h>

using namespace Kore;
using namespace Kore::Graphics5;

extern VkDevice device;
extern VkCommandPool cmd_pool;
extern PFN_vkQueuePresentKHR fpQueuePresentKHR;
extern PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
extern VkSwapchainKHR swapchain;
extern VkQueue queue;
extern VkFramebuffer* framebuffers;
extern VkRenderPass render_pass;
extern VkDescriptorSet desc_set;
extern Graphics5::Texture* vulkanTextures[8];
extern Graphics5::RenderTarget* vulkanRenderTargets[8];

struct SwapchainBuffers {
	VkImage image;
	VkCommandBuffer cmd;
	VkImageView view;
};

extern SwapchainBuffers* buffers;
extern uint32_t current_buffer;

namespace {
	VkCommandBuffer setup_cmd;
	bool began = false;
	bool onBackBuffer = false;
}

void demo_set_image_layout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout old_image_layout, VkImageLayout new_image_layout) {
	VkResult err;

	if (setup_cmd == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo cmd = {};
		cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd.pNext = nullptr;
		cmd.commandPool = cmd_pool;
		cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd.commandBufferCount = 1;

		err = vkAllocateCommandBuffers(device, &cmd, &setup_cmd);
		assert(!err);

		VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
		cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		cmd_buf_hinfo.pNext = nullptr;
		cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
		cmd_buf_hinfo.subpass = 0;
		cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
		cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
		cmd_buf_hinfo.queryFlags = 0;
		cmd_buf_hinfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo cmd_buf_info = {};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = nullptr;
		cmd_buf_info.flags = 0;
		cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

		err = vkBeginCommandBuffer(setup_cmd, &cmd_buf_info);
		assert(!err);
	}

	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.pNext = nullptr;
	image_memory_barrier.srcAccessMask = 0;
	image_memory_barrier.dstAccessMask = 0;
	image_memory_barrier.oldLayout = old_image_layout;
	image_memory_barrier.newLayout = new_image_layout;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange.aspectMask = aspectMask;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	if (old_image_layout != VK_IMAGE_LAYOUT_UNDEFINED) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		// Make sure anything that was copying from this image has completed
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		// Make sure any Copy or CPU writes to image are flushed
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}

	vkCmdPipelineBarrier(setup_cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
		&image_memory_barrier);
}

namespace {
	void demo_flush_init_cmd() {
		VkResult err;

		if (setup_cmd == VK_NULL_HANDLE) return;

		err = vkEndCommandBuffer(setup_cmd);
		assert(!err);

		const VkCommandBuffer cmd_bufs[] = { setup_cmd };
		VkFence nullFence = { VK_NULL_HANDLE };
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

	float depthStencil;
	float depthIncrement;
}

extern VkSemaphore presentCompleteSemaphore;

CommandList::CommandList() {
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = cmd_pool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	VkResult err = vkAllocateCommandBuffers(device, &cmd, &_buffer);
	assert(!err);

	_indexCount = 0;

	depthStencil = 1.0;
	depthIncrement = -0.01f;

	begin();
}

CommandList::~CommandList() {

}

void CommandList::begin() {
	if (began) return;
		
	// Assume the command buffer has been run on current_buffer before so
	// we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
	demo_set_image_layout(buffers[current_buffer].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	demo_flush_init_cmd();

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

	VkClearValue clear_values[2];
	memset(clear_values, 0, sizeof(VkClearValue) * 2);
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;
	clear_values[1].depthStencil.depth = depthStencil;
	clear_values[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo rp_begin = {};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = NULL;
	rp_begin.renderPass = render_pass;
	rp_begin.framebuffer = framebuffers[current_buffer];
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = System::windowWidth();
	rp_begin.renderArea.extent.height = System::windowHeight();
	rp_begin.clearValueCount = 2;
	rp_begin.pClearValues = clear_values;

	VkResult err = vkBeginCommandBuffer(_buffer, &cmd_buf_info);
	assert(!err);
	vkCmdBeginRenderPass(_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)System::windowWidth();
	viewport.height = (float)System::windowHeight();
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(_buffer, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = System::windowWidth();
	scissor.extent.height = System::windowHeight();
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(_buffer, 0, 1, &scissor);

	began = true;
	onBackBuffer = true;
}

void CommandList::end() {
	vkCmdEndRenderPass(_buffer);

	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.pNext = NULL;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	prePresentBarrier.image = buffers[current_buffer].image;
	VkImageMemoryBarrier* pmemory_barrier = &prePresentBarrier;
	vkCmdPipelineBarrier(_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);

	VkResult err = vkEndCommandBuffer(_buffer);
	assert(!err);

	VkFence nullFence = VK_NULL_HANDLE;
	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &presentCompleteSemaphore;
	submit_info.pWaitDstStageMask = &pipe_stage_flags;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &_buffer;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;

	err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
	assert(!err);

	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = NULL;
	present.swapchainCount = 1;
	present.pSwapchains = &swapchain;
	present.pImageIndices = &current_buffer;

	// TBD/TODO: SHOULD THE "present" PARAMETER BE "const" IN THE HEADER?
	err = fpQueuePresentKHR(queue, &present);
	/*if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		// demo->swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		// demo_resize(demo);
		error("VK_ERROR_OUT_OF_DATE_KHR");
	}
	else if (err == VK_SUBOPTIMAL_KHR) {
		// demo->swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
	}
	else {
		assert(!err);
	}*/
	assert(!err);

	err = vkQueueWaitIdle(queue);
	assert(err == VK_SUCCESS);

	vkDestroySemaphore(device, presentCompleteSemaphore, NULL);

	if (depthStencil > 0.99f) depthIncrement = -0.001f;
	if (depthStencil < 0.8f) depthIncrement = 0.001f;

	depthStencil += depthIncrement;

#ifndef KORE_WINDOWS
	vkDeviceWaitIdle(device);
#endif

	began = false;
}

void CommandList::clear(RenderTarget* renderTarget, uint flags, uint color, float depth, int stencil) {
	/*VkClearColorValue clearColor = {};
	clearColor.float32[0] = 1.0f;
	clearColor.float32[1] = 0.0f;
	clearColor.float32[2] = 0.0f;
	clearColor.float32[3] = 1.0f;
	VkImageSubresourceRange range = {};
	range.levelCount = 1;
	range.layerCount = 1;
	range.aspectMask = 0;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	vkCmdClearColorImage(draw_cmd, buffers[current_buffer].image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);*/
}

void CommandList::renderTargetToFramebufferBarrier(RenderTarget* renderTarget) {
	
}

void CommandList::framebufferToRenderTargetBarrier(RenderTarget* renderTarget) {
	
}

void CommandList::drawIndexedVertices() {
	drawIndexedVertices(0, _indexCount);
}

void CommandList::drawIndexedVertices(int start, int count) {
	vkCmdDrawIndexed(_buffer, count, 1, start, 0, 0);
}

void CommandList::viewport(int x, int y, int width, int height) {
	
}

void CommandList::scissor(int x, int y, int width, int height) {
	
}

void CommandList::disableScissor() {
	
}

void CommandList::setPipeline(PipelineState* pipeline) {
	_currentPipeline = pipeline;
	
	{
		uint8_t* data;
		VkResult err = vkMapMemory(device, _currentPipeline->memVertex, 0, _currentPipeline->mem_allocVertex.allocationSize, 0, (void**)&data);
		assert(!err);
		memcpy(data, &_currentPipeline->uniformDataVertex, sizeof(_currentPipeline->uniformDataVertex));
		vkUnmapMemory(device, _currentPipeline->memVertex);
	}

	{
		uint8_t* data;
		VkResult err = vkMapMemory(device, _currentPipeline->memFragment, 0, _currentPipeline->mem_allocFragment.allocationSize, 0, (void**)&data);
		assert(!err);
		memcpy(data, &_currentPipeline->uniformDataFragment, sizeof(_currentPipeline->uniformDataFragment));
		vkUnmapMemory(device, _currentPipeline->memFragment);
	}

	vkCmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentPipeline->pipeline);

	if (vulkanRenderTargets[0] != nullptr)
		vkCmdBindDescriptorSets(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentPipeline->pipeline_layout, 0, 1, &vulkanRenderTargets[0]->desc_set, 0, nullptr);
	else if (vulkanTextures[0] != nullptr)
		vkCmdBindDescriptorSets(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentPipeline->pipeline_layout, 0, 1, &vulkanTextures[0]->desc_set, 0, nullptr);
	else
		vkCmdBindDescriptorSets(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _currentPipeline->pipeline_layout, 0, 1, &desc_set, 0, nullptr);
}

void CommandList::setVertexBuffers(VertexBuffer** vertexBuffers, int* offsets_, int count) {
	vertexBuffers[0]->_set();
	VkDeviceSize offsets[1] = { offsets[0] * vertexBuffers[0]->count() * vertexBuffers[0]->stride() };
	vkCmdBindVertexBuffers(_buffer, 0, 1, &vertexBuffers[0]->vertices.buf, offsets);
}

void CommandList::setIndexBuffer(IndexBuffer& indexBuffer) {
	_indexCount = indexBuffer.count();
	vkCmdBindIndexBuffer(_buffer, indexBuffer.buf, 0, VK_INDEX_TYPE_UINT32);
}

void setImageLayout(VkCommandBuffer _buffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

namespace {
	Graphics5::RenderTarget* currentRenderTarget = nullptr;

	void endPass(VkCommandBuffer _buffer) {
		vkCmdEndRenderPass(_buffer);

		if (currentRenderTarget == nullptr) {
			/*VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = NULL;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barrier.image = buffers[current_buffer].image;

			vkCmdPipelineBarrier(draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);*/
		}
		else {
			setImageLayout(_buffer, currentRenderTarget->sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			setImageLayout(_buffer, currentRenderTarget->destImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkImageBlit imgBlit;

			imgBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgBlit.srcSubresource.mipLevel = 0;
			imgBlit.srcSubresource.baseArrayLayer = 0;
			imgBlit.srcSubresource.layerCount = 1;

			imgBlit.srcOffsets[0] = { 0, 0, 0 };
			imgBlit.srcOffsets[1].x = currentRenderTarget->width;
			imgBlit.srcOffsets[1].y = currentRenderTarget->height;
			imgBlit.srcOffsets[1].z = 1;

			imgBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgBlit.dstSubresource.mipLevel = 0;
			imgBlit.dstSubresource.baseArrayLayer = 0;
			imgBlit.dstSubresource.layerCount = 1;

			imgBlit.dstOffsets[0] = { 0, 0, 0 };
			imgBlit.dstOffsets[1].x = currentRenderTarget->width;
			imgBlit.dstOffsets[1].y = currentRenderTarget->height;
			imgBlit.dstOffsets[1].z = 1;

			vkCmdBlitImage(_buffer, currentRenderTarget->sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, currentRenderTarget->destImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgBlit, VK_FILTER_LINEAR);

			setImageLayout(_buffer, currentRenderTarget->sourceImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			setImageLayout(_buffer, currentRenderTarget->destImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		/*VkResult err = vkEndCommandBuffer(draw_cmd);
		assert(!err);

		VkFence nullFence = VK_NULL_HANDLE;
		VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = NULL;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &presentCompleteSemaphore;
		submit_info.pWaitDstStageMask = &pipe_stage_flags;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &draw_cmd;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = NULL;

		err = vkQueueSubmit(queue, 1, &submit_info, nullFence);
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

		VkClearValue clear_values[2];
		memset(clear_values, 0, sizeof(VkClearValue) * 2);
		clear_values[0].color.float32[0] = 0.0f;
		clear_values[0].color.float32[1] = 0.0f;
		clear_values[0].color.float32[2] = 0.0f;
		clear_values[0].color.float32[3] = 1.0f;
		clear_values[1].depthStencil.depth = depthStencil;
		clear_values[1].depthStencil.stencil = 0;

		VkRenderPassBeginInfo rp_begin = {};
		rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext = NULL;
		rp_begin.renderPass = render_pass;
		rp_begin.framebuffer = framebuffers[current_buffer];
		rp_begin.renderArea.offset.x = 0;
		rp_begin.renderArea.offset.y = 0;
		rp_begin.renderArea.extent.width = width;
		rp_begin.renderArea.extent.height = height;
		rp_begin.clearValueCount = 2;
		rp_begin.pClearValues = clear_values;

		err = vkBeginCommandBuffer(draw_cmd, &cmd_buf_info);
		assert(!err);*/
	}
}

void CommandList::setRenderTargets(RenderTarget** targets, int count) {
	endPass(_buffer);

	currentRenderTarget = targets[0];
	onBackBuffer = false;

	VkClearValue clear_values[1];
	memset(clear_values, 0, sizeof(VkClearValue));
	clear_values[0].color.float32[0] = 0.0f;
	clear_values[0].color.float32[1] = 0.0f;
	clear_values[0].color.float32[2] = 0.0f;
	clear_values[0].color.float32[3] = 1.0f;

	VkRenderPassBeginInfo rp_begin = {};
	rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin.pNext = nullptr;
	rp_begin.renderPass = targets[0]->renderPass;
	rp_begin.framebuffer = targets[0]->framebuffer;
	rp_begin.renderArea.offset.x = 0;
	rp_begin.renderArea.offset.y = 0;
	rp_begin.renderArea.extent.width = targets[0]->width;
	rp_begin.renderArea.extent.height = targets[0]->height;
	rp_begin.clearValueCount = 1;
	rp_begin.pClearValues = clear_values;

	vkCmdBeginRenderPass(_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	memset(&viewport, 0, sizeof(viewport));
	viewport.width = (float)targets[0]->width;
	viewport.height = (float)targets[0]->height;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(_buffer, 0, 1, &viewport);

	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = targets[0]->width;
	scissor.extent.height = targets[0]->height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(_buffer, 0, 1, &scissor);
}

void CommandList::upload(IndexBuffer* buffer) {
	
}

void CommandList::upload(Texture* texture) {
	
}
