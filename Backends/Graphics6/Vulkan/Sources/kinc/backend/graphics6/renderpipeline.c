#include "vulkan.h"
#include <kinc/graphics6/renderpipeline.h>
#include <kinc/graphics6/shader.h>

#include <malloc.h>

#define F(k, v)                                                                                                                                                \
	case k:                                                                                                                                                    \
		return v;
VkFormat convert_texture_format(kinc_g6_texture_format_t format) {
	switch (format) {
		F(KINC_G6_TEXTURE_FORMAT_NONE, VK_FORMAT_UNDEFINED)

		F(KINC_G6_TEXTURE_FORMAT_R8_UNORM, VK_FORMAT_R8_UNORM)
		F(KINC_G6_TEXTURE_FORMAT_R8_SNORM, VK_FORMAT_R8_SNORM)
		F(KINC_G6_TEXTURE_FORMAT_R8_UINT, VK_FORMAT_R8_UINT)
		F(KINC_G6_TEXTURE_FORMAT_R8_SINT, VK_FORMAT_R8_SINT)

		F(KINC_G6_TEXTURE_FORMAT_R16_UINT, VK_FORMAT_R16_UINT)
		F(KINC_G6_TEXTURE_FORMAT_R16_SINT, VK_FORMAT_R16_SINT)
		F(KINC_G6_TEXTURE_FORMAT_R16_FLOAT, VK_FORMAT_R16_SFLOAT)
		F(KINC_G6_TEXTURE_FORMAT_RG8_UNORM, VK_FORMAT_R8G8_UNORM)
		F(KINC_G6_TEXTURE_FORMAT_RG8_SNORM, VK_FORMAT_R8G8_SNORM)
		F(KINC_G6_TEXTURE_FORMAT_RG8_UINT, VK_FORMAT_R8G8_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RG8_SINT, VK_FORMAT_R8G8_SINT)

		F(KINC_G6_TEXTURE_FORMAT_R32_UINT, VK_FORMAT_R32_UINT)
		F(KINC_G6_TEXTURE_FORMAT_R32_SINT, VK_FORMAT_R32_SINT)
		F(KINC_G6_TEXTURE_FORMAT_R32_FLOAT, VK_FORMAT_R32_SFLOAT)
		F(KINC_G6_TEXTURE_FORMAT_RG16_UINT, VK_FORMAT_R16G16_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RG16_SINT, VK_FORMAT_R16G16_SINT)
		F(KINC_G6_TEXTURE_FORMAT_RG16_FLOAT, VK_FORMAT_R16G16_SFLOAT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA8_UNORM, VK_FORMAT_R8G8B8A8_UNORM)
		F(KINC_G6_TEXTURE_FORMAT_RGBA8_UNORM_SRGB, VK_FORMAT_R8G8B8A8_SRGB)
		F(KINC_G6_TEXTURE_FORMAT_RGBA8_SNORM, VK_FORMAT_R8G8B8A8_SNORM)
		F(KINC_G6_TEXTURE_FORMAT_RGBA8_UINT, VK_FORMAT_R8G8B8A8_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA8_SINT, VK_FORMAT_R8G8B8A8_SINT)
		F(KINC_G6_TEXTURE_FORMAT_BGRA8_UNORM, VK_FORMAT_B8G8R8A8_UNORM)
		F(KINC_G6_TEXTURE_FORMAT_BGRA8_UNORM_SRGB, VK_FORMAT_B8G8R8A8_SRGB)

		F(KINC_G6_TEXTURE_FORMAT_RGB9E5_UFLOAT, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
		F(KINC_G6_TEXTURE_FORMAT_RGB10A2_UNORM, VK_FORMAT_A2R10G10B10_UNORM_PACK32)
		F(KINC_G6_TEXTURE_FORMAT_RG11B10_UFLOAT, VK_FORMAT_B10G11R11_UFLOAT_PACK32)

		F(KINC_G6_TEXTURE_FORMAT_RG32_UINT, VK_FORMAT_R32G32_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RG32_SINT, VK_FORMAT_R32G32_SINT)
		F(KINC_G6_TEXTURE_FORMAT_RG32_FLOAT, VK_FORMAT_R32G32_SFLOAT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA16_UINT, VK_FORMAT_R16G16B16A16_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA16_SINT, VK_FORMAT_R16G16B16A16_SINT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA16_FLOAT, VK_FORMAT_R16G16B16A16_SFLOAT)

		F(KINC_G6_TEXTURE_FORMAT_RGBA32_UINT, VK_FORMAT_R32G32B32_UINT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA32_SINT, VK_FORMAT_R32G32B32_SINT)
		F(KINC_G6_TEXTURE_FORMAT_RGBA32_FLOAT, VK_FORMAT_R32G32B32_SFLOAT)

		F(KINC_G6_TEXTURE_FORMAT_STENCIL8, VK_FORMAT_S8_UINT)
		F(KINC_G6_TEXTURE_FORMAT_DEPTH16_UNORM, VK_FORMAT_D16_UNORM)
		F(KINC_G6_TEXTURE_FORMAT_DEPTH24_PLUS, VK_FORMAT_X8_D24_UNORM_PACK32) // ??
		F(KINC_G6_TEXTURE_FORMAT_DEPTH24_PLUS_STENCIL8, VK_FORMAT_D24_UNORM_S8_UINT)
		F(KINC_G6_TEXTURE_FORMAT_DEPTH32_FLOAT, VK_FORMAT_D32_SFLOAT)
	}
}

VkFormat convert_vertex_format(kinc_g6_vertex_format_t format) {
	switch (format) {
		F(KINC_G6_VERTEX_FORMAT_UINT8X2, VK_FORMAT_R8G8_UINT)
		F(KINC_G6_VERTEX_FORMAT_UINT8X4, VK_FORMAT_R8G8B8A8_UINT)
		F(KINC_G6_VERTEX_FORMAT_SINT8X2, VK_FORMAT_R8G8_SINT)
		F(KINC_G6_VERTEX_FORMAT_SINT8X4, VK_FORMAT_R8G8B8A8_SINT)
		F(KINC_G6_VERTEX_FORMAT_UNORM8X2, VK_FORMAT_R8G8_UNORM)
		F(KINC_G6_VERTEX_FORMAT_UNORM8X4, VK_FORMAT_R8G8B8A8_UNORM)
		F(KINC_G6_VERTEX_FORMAT_SNORM8X2, VK_FORMAT_R8G8_SNORM)
		F(KINC_G6_VERTEX_FORMAT_SNORM8X4, VK_FORMAT_R8G8B8A8_SNORM)
		F(KINC_G6_VERTEX_FORMAT_UINT16X2, VK_FORMAT_R16G16_UINT)
		F(KINC_G6_VERTEX_FORMAT_UINT16X4, VK_FORMAT_R16G16B16A16_UINT)
		F(KINC_G6_VERTEX_FORMAT_SINT16X2, VK_FORMAT_R16G16_SINT)
		F(KINC_G6_VERTEX_FORMAT_SINT16X4, VK_FORMAT_R16G16B16A16_SINT)
		F(KINC_G6_VERTEX_FORMAT_UNORM16X2, VK_FORMAT_R16G16_UNORM)
		F(KINC_G6_VERTEX_FORMAT_UNORM16X4, VK_FORMAT_R16G16B16A16_UNORM)
		F(KINC_G6_VERTEX_FORMAT_SNORM16X2, VK_FORMAT_R16G16_SNORM)
		F(KINC_G6_VERTEX_FORMAT_SNORM16X4, VK_FORMAT_R16G16B16A16_SNORM)
		F(KINC_G6_VERTEX_FORMAT_FLOAT16X2, VK_FORMAT_R16G16_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_FLOAT16X4, VK_FORMAT_R16G16B16A16_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_FLOAT32, VK_FORMAT_R32_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_FLOAT32X2, VK_FORMAT_R32G32_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_FLOAT32X3, VK_FORMAT_R32G32B32_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_FLOAT32X4, VK_FORMAT_R32G32B32A32_SFLOAT)
		F(KINC_G6_VERTEX_FORMAT_UINT32, VK_FORMAT_R32_UINT)
		F(KINC_G6_VERTEX_FORMAT_UINT32X2, VK_FORMAT_R32G32_UINT)
		F(KINC_G6_VERTEX_FORMAT_UINT32X3, VK_FORMAT_R32G32B32_UINT)
		F(KINC_G6_VERTEX_FORMAT_UINT32X4, VK_FORMAT_R32G32B32A32_UINT)
		F(KINC_G6_VERTEX_FORMAT_SINT32, VK_FORMAT_R32_SINT)
		F(KINC_G6_VERTEX_FORMAT_SINT32X2, VK_FORMAT_R32G32_SINT)
		F(KINC_G6_VERTEX_FORMAT_SINT32X3, VK_FORMAT_R32G32B32_SINT)
		F(KINC_G6_VERTEX_FORMAT_SINT32X4, VK_FORMAT_R32G32B32A32_SINT)
	}
}

VkPolygonMode convert_polygon_mode(kinc_g6_primitive_topology_t topology) {
	switch (topology) {
	case KINC_G6_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return VK_POLYGON_MODE_POINT;
	case KINC_G6_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case KINC_G6_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return VK_POLYGON_MODE_LINE;
	case KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return VK_POLYGON_MODE_FILL;
	}
}

VkPrimitiveTopology convert_topology(kinc_g6_primitive_topology_t topology) {
	switch (topology) {
	case KINC_G6_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case KINC_G6_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case KINC_G6_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}
}

VkCullModeFlags convert_cull_mode(kinc_g6_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case KINC_G6_CULL_MODE_NONE:
		return VK_CULL_MODE_NONE;
	case KINC_G6_CULL_MODE_FRONT:
		return VK_CULL_MODE_FRONT_BIT;
	case KINC_G6_CULL_MODE_BACK:
		return VK_CULL_MODE_BACK_BIT;
	}
}

VkFrontFace convert_front_face(kinc_g6_front_face_t front_face) {
	switch (front_face) {
	case KINC_G6_FRONT_FACE_CCW:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	case KINC_G6_FRONT_FACE_CW:
		return VK_FRONT_FACE_CLOCKWISE;
	}
}

VkCompareOp convert_compare(kinc_g6_compare_function_t compare) {
	switch (compare) {
	case KINC_G6_COMPARE_FUNCTION_NEVER:
		return VK_COMPARE_OP_NEVER;
	case KINC_G6_COMPARE_FUNCTION_LESS:
		return VK_COMPARE_OP_LESS;
	case KINC_G6_COMPARE_FUNCTION_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case KINC_G6_COMPARE_FUNCTION_LESS_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case KINC_G6_COMPARE_FUNCTION_GREATER:
		return VK_COMPARE_OP_GREATER;
	case KINC_G6_COMPARE_FUNCTION_NOT_EQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case KINC_G6_COMPARE_FUNCTION_GREATER_EQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case KINC_G6_COMPARE_FUNCTION_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	}
}

VkStencilOp convert_stencil(kinc_g6_stencil_operation_t stencil) {
	switch (stencil) {
	case KINC_G6_STENCIL_OPERATION_KEEP:
		return VK_STENCIL_OP_KEEP;
	case KINC_G6_STENCIL_OPERATION_ZERO:
		return VK_STENCIL_OP_ZERO;
	case KINC_G6_STENCIL_OPERATION_REPLACE:
		return VK_STENCIL_OP_REPLACE;
	case KINC_G6_STENCIL_OPERATION_INVERT:
		return VK_STENCIL_OP_INVERT;
	case KINC_G6_STENCIL_OPERATION_INCREMENT_CLAMP:
		return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case KINC_G6_STENCIL_OPERATION_DECREMENT_CLAMP:
		return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case KINC_G6_STENCIL_OPERATION_INCREMENT_WRAP:
		return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case KINC_G6_STENCIL_OPERATION_DECREMENT_WRAP:
		return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
}

VkBlendOp convert_blend_op(kinc_g6_blend_operation_t op) {
	switch (op) {
	case KINC_G6_BLEND_OPERATION_ADD:
		return VK_BLEND_OP_ADD;
	case KINC_G6_BLEND_OPERATION_SUBSTRACT:
		return VK_BLEND_OP_SUBTRACT;
	case KINC_G6_BLEND_OPERATION_REVERSE_SUBSTRACT:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case KINC_G6_BLEND_OPERATION_MIN:
		return VK_BLEND_OP_MIN;
	case KINC_G6_BLEND_OPERATION_MAX:
		return VK_BLEND_OP_MAX;
	}
}

VkBlendFactor convert_blend_factor(kinc_g6_blend_factor_t factor) {
	switch (factor) {
	case KINC_G6_BLEND_FACTOR_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case KINC_G6_BLEND_FACTOR_ONE:
		return VK_BLEND_FACTOR_ONE;
	case KINC_G6_BLEND_FACTOR_SRC:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case KINC_G6_BLEND_FACTOR_ONE_MINUS_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
	case KINC_G6_BLEND_FACTOR_SRC_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case KINC_G6_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case KINC_G6_BLEND_FACTOR_DST:
		return VK_BLEND_FACTOR_DST_COLOR;
	case KINC_G6_BLEND_FACTOR_ONE_MINUS_DST:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case KINC_G6_BLEND_FACTOR_DST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case KINC_G6_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case KINC_G6_BLEND_FACTOR_SRC_ALPHA_SATURATED:
		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	case KINC_G6_BLEND_FACTOR_CONSTANT:
		return VK_BLEND_FACTOR_CONSTANT_COLOR;
	case KINC_G6_BLEND_FACTOR_ONE_MINUS_CONSTANT:
		return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	}
}
#undef F

void kinc_g6_render_pipeline_init(kinc_g6_render_pipeline_t *pipeline, const kinc_g6_render_pipeline_descriptor_t *descriptor) {
	VkPipelineShaderStageCreateInfo shader_stages[2] = {0};
	{
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].pNext = NULL;
		shader_stages[0].flags = 0;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = descriptor->vertex_shader->impl.module;
		shader_stages[0].pName = "main";
		shader_stages[0].pSpecializationInfo = NULL;
	}
	{
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].pNext = NULL;
		shader_stages[1].flags = 0;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = descriptor->fragment_shader->impl.module;
		shader_stages[1].pName = "main";
		shader_stages[1].pSpecializationInfo = NULL;
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_state = {0};

	vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state.pNext = 0;
	vertex_input_state.flags = 0;
	uint32_t vertex_binding_count = 0;
	uint32_t vertex_attribute_count = 0;
	for (int i = 0; descriptor->input_layout[i] != NULL; i++) {
		vertex_binding_count++;
		vertex_attribute_count += descriptor->input_layout[i]->attribute_count;
	}
	VkVertexInputBindingDescription *vertex_bindings = malloc(sizeof(VkVertexInputBindingDescription) * vertex_binding_count);
	VkVertexInputAttributeDescription *vertex_attributes = malloc(sizeof(VkVertexInputAttributeDescription) * vertex_attribute_count);

	uint32_t attr_index = 0;
	for (int i = 0; descriptor->input_layout[i] != NULL; i++) {
		vertex_bindings[i].binding = i;
		vertex_bindings[i].inputRate = descriptor->input_layout[i]->instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		vertex_bindings[i].stride = descriptor->input_layout[i]->stride;
		for (int idx = 0; idx < descriptor->input_layout[i]->attribute_count; idx++) {
			vertex_attributes[attr_index].binding = i;
			vertex_attributes[attr_index].format = convert_vertex_format(descriptor->input_layout[i]->attributes[idx].format);
			vertex_attributes[attr_index].location = descriptor->input_layout[i]->attributes[idx].location;
			vertex_attributes[attr_index].offset = descriptor->input_layout[i]->attributes[idx].offset;
			attr_index++;
		}
	}
	vertex_input_state.vertexBindingDescriptionCount = vertex_binding_count;
	vertex_input_state.vertexAttributeDescriptionCount = vertex_attribute_count;
	vertex_input_state.pVertexBindingDescriptions = vertex_bindings;
	vertex_input_state.pVertexAttributeDescriptions = vertex_attributes;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {0};
	input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state.pNext = NULL;
	input_assembly_state.flags = 0;
	input_assembly_state.topology = convert_topology(descriptor->topology);
	input_assembly_state.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state = {0};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = NULL;
	viewport_state.flags = 0;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = NULL; // Always dynamic
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = NULL; // Always dynamic

	VkPipelineRasterizationStateCreateInfo raster_state = {0};
	raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_state.pNext = NULL;
	raster_state.flags = 0;

	raster_state.depthClampEnable = VK_FALSE; // descriptor->clamp_depth;
	raster_state.rasterizerDiscardEnable = VK_FALSE;
	raster_state.polygonMode = convert_polygon_mode(descriptor->topology);
	raster_state.cullMode = convert_cull_mode(descriptor->cull_mode);
	raster_state.frontFace = convert_front_face(descriptor->front_face);
	raster_state.depthBiasEnable = VK_FALSE;
	raster_state.depthBiasConstantFactor = (float)descriptor->depth_bias;
	raster_state.depthBiasClamp = descriptor->depth_bias_clamp;
	raster_state.depthBiasSlopeFactor = descriptor->depth_bias_slope_scale;
	raster_state.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisample_state = {0};
	multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state.pNext = NULL;
	multisample_state.flags = 0;
	multisample_state.rasterizationSamples = descriptor->sample_count;
	multisample_state.sampleShadingEnable = VK_FALSE;
	multisample_state.minSampleShading = 0;
	multisample_state.pSampleMask = NULL;
	multisample_state.alphaToCoverageEnable = descriptor->sample_alpha_to_coverage_enabled;
	multisample_state.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {0};
	depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state.pNext = NULL;
	depth_stencil_state.flags = 0;
	depth_stencil_state.depthTestEnable = descriptor->enable_depth;
	depth_stencil_state.depthWriteEnable = descriptor->depth_write;
	depth_stencil_state.depthCompareOp = convert_compare(descriptor->depth_compare);
	depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state.stencilTestEnable = descriptor->enable_stencil;

	depth_stencil_state.front.failOp = convert_stencil(descriptor->stencil_fail_front);
	depth_stencil_state.front.passOp = convert_stencil(descriptor->stencil_pass_front);
	depth_stencil_state.front.depthFailOp = convert_stencil(descriptor->stencil_fail_front);
	depth_stencil_state.front.compareOp = convert_compare(descriptor->stencil_compare_front);
	depth_stencil_state.front.compareMask = descriptor->stencil_read_mask;
	depth_stencil_state.front.writeMask = descriptor->stencil_write_mask;
	depth_stencil_state.front.reference = descriptor->stencil_reference;

	depth_stencil_state.back.failOp = convert_stencil(descriptor->stencil_fail_back);
	depth_stencil_state.back.passOp = convert_stencil(descriptor->stencil_pass_back);
	depth_stencil_state.back.depthFailOp = convert_stencil(descriptor->stencil_fail_back);
	depth_stencil_state.back.compareOp = convert_compare(descriptor->stencil_compare_back);
	depth_stencil_state.back.compareMask = descriptor->stencil_read_mask;
	depth_stencil_state.back.writeMask = descriptor->stencil_write_mask;
	depth_stencil_state.back.reference = descriptor->stencil_reference;

	depth_stencil_state.minDepthBounds = 0.0f;
	depth_stencil_state.maxDepthBounds = 0.0f;

	VkPipelineColorBlendStateCreateInfo blend_state = {0};
	blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_state.pNext = NULL;
	blend_state.flags = 0;
	blend_state.logicOpEnable = VK_FALSE;
	blend_state.logicOp = VK_LOGIC_OP_NO_OP;
	blend_state.attachmentCount = descriptor->attachment_count;
	VkPipelineColorBlendAttachmentState attachments[8] = {0};
	for (int i = 0; i < descriptor->attachment_count; i++) {
		attachments[i].blendEnable = descriptor->attachments[i].blend_enable;
		attachments[i].srcColorBlendFactor = convert_blend_factor(descriptor->attachments[i].blend_src);
		attachments[i].dstColorBlendFactor = convert_blend_factor(descriptor->attachments[i].blend_dst);
		attachments[i].colorBlendOp = convert_blend_op(descriptor->attachments[i].operation);
		attachments[i].srcAlphaBlendFactor = convert_blend_factor(descriptor->attachments[i].alpha_blend_src);
		attachments[i].dstAlphaBlendFactor = convert_blend_factor(descriptor->attachments[i].alpha_blend_dst);
		attachments[i].alphaBlendOp = convert_blend_op(descriptor->attachments[i].alpha_operation);
		attachments[i].colorWriteMask = 0;
		if (descriptor->attachments[i].color_write_mask_red) attachments[i].colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if (descriptor->attachments[i].color_write_mask_green) attachments[i].colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if (descriptor->attachments[i].color_write_mask_blue) attachments[i].colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if (descriptor->attachments[i].color_write_mask_alpha) attachments[i].colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
	}
	blend_state.pAttachments = attachments;

	VkPipelineDynamicStateCreateInfo dynamic_state = {0};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pNext = NULL;
	dynamic_state.flags = 0;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkGraphicsPipelineCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.stageCount = 2;
	create_info.pStages = shader_stages;

	create_info.pVertexInputState = &vertex_input_state;
	create_info.pInputAssemblyState = &input_assembly_state;
	create_info.pViewportState = &viewport_state;
	create_info.pRasterizationState = &raster_state;
	create_info.pMultisampleState = &multisample_state;
	create_info.pDepthStencilState = &depth_stencil_state;
	create_info.pColorBlendState = &blend_state;
	create_info.pDynamicState = &dynamic_state;
	create_info.layout = descriptor->layout->impl.layout;

	{
		VkRenderPassCreateInfo rp_info = {0};
		rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext = 0;
		rp_info.flags = 0;
		rp_info.attachmentCount = 1;
		VkAttachmentDescription attachments[8] = {0};
		for (int i = 0; i < descriptor->attachment_count; i++) {
			attachments[i].flags = 0;
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			attachments[i].format = convert_texture_format(descriptor->attachments[0].format);
			attachments[i].samples = descriptor->sample_count;
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		rp_info.pAttachments = attachments;

		rp_info.subpassCount = 1;
		VkSubpassDescription subpass = {0};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.colorAttachmentCount = descriptor->attachment_count;
		VkAttachmentReference references[8] = {0};
		for (int i = 0; i < 8; i++) {
			references[i].attachment = i;
			references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		subpass.pColorAttachments = references;
		subpass.pDepthStencilAttachment = &(VkAttachmentReference){.attachment = VK_ATTACHMENT_UNUSED};
		rp_info.pSubpasses = &subpass;
		vkCreateRenderPass(context.device, &rp_info, NULL, &create_info.renderPass);
	}

	create_info.subpass = 0;
	create_info.basePipelineHandle = VK_NULL_HANDLE;
	create_info.basePipelineIndex = -1;
	CHECK(vkCreateGraphicsPipelines(context.device, context.pipeline_cache, 1, &create_info, NULL, &pipeline->impl.pipeline));
	free(vertex_bindings);
	free(vertex_attributes);
}

void kinc_g6_render_pipeline_destroy(kinc_g6_render_pipeline_t *pipeline) {
	vkDestroyPipeline(context.device, pipeline->impl.pipeline, NULL);
}

void kinc_g6_pipeline_layout_init(kinc_g6_pipeline_layout_t *layout, const kinc_g6_pipeline_layout_descriptor_t *descriptor) {
	VkPipelineLayoutCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.setLayoutCount = descriptor->count;
	VkDescriptorSetLayout *layouts = alloca(sizeof(*create_info.pSetLayouts) * create_info.setLayoutCount);
	for (int i = 0; i < create_info.setLayoutCount; i++) {
		layouts[i] = descriptor->bind_groups[i]->impl.layout;
	}
	create_info.pSetLayouts = layouts;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;
	vkCreatePipelineLayout(context.device, &create_info, NULL, &layout->impl.layout);
}

void kinc_g6_pipeline_layout_destroy(kinc_g6_pipeline_layout_t *layout) {
	vkDestroyPipelineLayout(context.device, layout->impl.layout, NULL);
}
