#include <kinc/graphics6/commandbuffer.h>
#include <kinc/graphics6/renderpipeline.h>
#include <kinc/graphics6/bindgroup.h>

VkAttachmentLoadOp convert_load_op(kinc_g6_load_op_t load) {
	switch (load) {
	case KINC_G6_LOAD_OP_LOAD:
		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case KINC_G6_LOAD_OP_CLEAR:
		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case KINC_G6_LOAD_OP_DONT_CARE:
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp convert_store_op(kinc_g6_store_op_t store) {
	switch (store) {
	case KINC_G6_STORE_OP_STORE:
		return VK_ATTACHMENT_STORE_OP_STORE;
	case KINC_G6_STORE_OP_CLEAR:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	case KINC_G6_STORE_OP_DONT_CARE:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

// Initialize the command buffer
void kinc_g6_command_buffer_init(kinc_g6_command_buffer_t *buffer) {
	VkCommandBufferAllocateInfo alloc_info = {0};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.commandPool = context.command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	CHECK(vkAllocateCommandBuffers(context.device, &alloc_info, &buffer->impl.buffer));
}
// Begin recording commands
void kinc_g6_command_buffer_begin(kinc_g6_command_buffer_t *buffer) {
	VkCommandBufferBeginInfo begin_info = {0};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.pInheritanceInfo = NULL;
	begin_info.flags = 0;
	CHECK(vkBeginCommandBuffer(buffer->impl.buffer, &begin_info));
}
// Stop recording commands
void kinc_g6_command_buffer_end(kinc_g6_command_buffer_t *buffer) {
	CHECK(vkEndCommandBuffer(buffer->impl.buffer));
}

void kinc_g6_command_buffer_render_pass_begin(kinc_g6_command_buffer_t *buffer, const kinc_g6_render_pass_descriptor_t *descriptor) {
	VkRenderPassBeginInfo begin_info = {0};
	begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.renderArea.offset.x = 0;
	begin_info.renderArea.offset.y = 0;
	begin_info.renderArea.extent.width = descriptor->color_attachments[0].texture->impl.extent.width;
	begin_info.renderArea.extent.height = descriptor->color_attachments[0].texture->impl.extent.height;
	{
		VkRenderPassCreateInfo rp_info = {0};
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext = 0;
		rp_info.flags = 0;
		rp_info.attachmentCount = 1;
		VkAttachmentDescription attachments[8] = {0};
		for (int i = 0; i < descriptor->color_attachment_count; i++) {
			attachments[i].flags = 0;
			attachments[i].loadOp = convert_load_op(descriptor->color_attachments[i].load_op);
			attachments[i].storeOp = convert_store_op(descriptor->color_attachments[i].store_op);
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			attachments[i].format = descriptor->color_attachments[i].texture->impl.format;
			attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		rp_info.pAttachments = attachments;

		rp_info.subpassCount = 1;
		VkSubpassDescription subpass = {0};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.colorAttachmentCount = descriptor->color_attachment_count;
		VkAttachmentReference references[8] = {0};
		for (int i = 0; i < 8; i++) {
			references[i].attachment = i;
			references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		subpass.pColorAttachments = references;
		subpass.pDepthStencilAttachment = &(VkAttachmentReference){.attachment = VK_ATTACHMENT_UNUSED};
		rp_info.pSubpasses = &subpass;
		rp_info.dependencyCount = 1;
		VkSubpassDependency dependency = {0};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		rp_info.pDependencies = &dependency;
		vkCreateRenderPass(context.device, &rp_info, NULL, &begin_info.renderPass);
	}
	{
		VkFramebufferCreateInfo fb_info = {0};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = NULL;
		fb_info.flags = 0;
		fb_info.attachmentCount = descriptor->color_attachment_count;
		VkImageView views[8] = {0};
		for (int i = 0; i < fb_info.attachmentCount; i++) {
			views[i] = descriptor->color_attachments[i].texture->impl.view;
		}
		fb_info.pAttachments = views;
		fb_info.renderPass = begin_info.renderPass;
		fb_info.width = descriptor->color_attachments[0].texture->impl.extent.width;
		fb_info.height = descriptor->color_attachments[0].texture->impl.extent.height;
		fb_info.layers = 1;

		CHECK(vkCreateFramebuffer(context.device, &fb_info, NULL, &begin_info.framebuffer));
	}
	begin_info.clearValueCount = 0;
	VkClearValue clears[8] = {0};
	for (int i = 0; i < descriptor->color_attachment_count; i++)
		if (descriptor->color_attachments[i].load_op == KINC_G6_LOAD_OP_CLEAR) {
			begin_info.clearValueCount++;
			clears[i].color.float32[0] = 0;
			clears[i].color.float32[1] = 0;
			clears[i].color.float32[2] = 0;
			clears[i].color.float32[3] = 0;
		}
	begin_info.pClearValues = clears;
	vkCmdBeginRenderPass(buffer->impl.buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
	kinc_g6_command_buffer_set_scissor(buffer, 0, 0, begin_info.renderArea.extent.width, begin_info.renderArea.extent.height);
	kinc_g6_command_buffer_set_viewport(buffer, 0, 0, begin_info.renderArea.extent.width, begin_info.renderArea.extent.height, 0, 1);
}
void kinc_g6_command_buffer_render_pass_end(kinc_g6_command_buffer_t *buffer) {
	vkCmdEndRenderPass(buffer->impl.buffer);
}

void kinc_g6_command_buffer_set_render_pipeline(kinc_g6_command_buffer_t *buffer, kinc_g6_render_pipeline_t *pipeline) {
	vkCmdBindPipeline(buffer->impl.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->impl.pipeline);
}

void kinc_g6_command_buffer_set_index_buffer(kinc_g6_command_buffer_t *buffer, kinc_g6_buffer_t *index_buffer, int offset) {
	vkCmdBindIndexBuffer(buffer->impl.buffer, index_buffer->impl.buffer, offset, VK_INDEX_TYPE_UINT32);
}
void kinc_g6_command_buffer_set_vertex_buffers(kinc_g6_command_buffer_t *buffer, kinc_g6_buffer_t **vertex_buffers, int *offsets, int count) {
	VkBuffer pBuffers[16];
	VkDeviceSize pOffsets[16];
	for (int i = 0; i < count; i++) {
		pBuffers[i] = vertex_buffers[i]->impl.buffer;
		pOffsets[i] = offsets[i];
	}
	vkCmdBindVertexBuffers(buffer->impl.buffer, 0, count, pBuffers, pOffsets);
}

void kinc_g6_command_buffer_set_bind_group(kinc_g6_command_buffer_t *buffer, int index, struct kinc_g6_bind_group *group, uint32_t dynamicOffsetsCount,
                                           uint32_t *dynamicOffsets) {
	vkCmdBindDescriptorSets(buffer->impl.buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VK_NULL_HANDLE, index, 1, &group->impl.set, dynamicOffsetsCount,
	                        dynamicOffsets);
}

void kinc_g6_command_buffer_set_viewport(kinc_g6_command_buffer_t *buffer, int x, int y, int width, int height, int min_depth, int max_depth) {
	VkViewport viewport = {0};
	viewport.x = x;
	viewport.y = y;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = min_depth;
	viewport.maxDepth = max_depth;
	vkCmdSetViewport(buffer->impl.buffer, 0, 1, &viewport);
}

void kinc_g6_command_buffer_set_scissor(kinc_g6_command_buffer_t *buffer, int x, int y, int width, int height) {
	VkRect2D scissor = {0};
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = width;
	scissor.extent.height = height;
	vkCmdSetScissor(buffer->impl.buffer, 0, 1, &scissor);
}

//  void kinc_g6_command_buffer_set_blend_constant(kinc_g6_command_buffer_t* buffer, uint32_t constant);
//  void kinc_g6_command_buffer_set_stencil_reference(kinc_g6_command_buffer_t *buffer,)

void kinc_g6_command_buffer_draw_indexed_vertices(kinc_g6_command_buffer_t *buffer, int start, int count, int vertex_offset) {
	vkCmdDrawIndexed(buffer->impl.buffer, count, 1, start, vertex_offset, 0);
}

void kinc_g6_command_list_draw_indexed_vertices_instanced(kinc_g6_command_buffer_t *buffer, int instance_count, int start, int count) {
	vkCmdDrawIndexed(buffer->impl.buffer, count, instance_count, start, 0, 0);
}

#ifdef __cplusplus
}
#endif