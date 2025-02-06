#include "pipeline_functions.h"
#include "pipeline_structs.h"

#include <kinc/log.h>

void kope_vulkan_render_pipeline_init(kope_vulkan_device *device, kope_vulkan_render_pipeline *pipe, const kope_vulkan_render_pipeline_parameters *parameters) {
	/*memset(pipeline->impl.vertexLocations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.vertexOffsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentLocations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.fragmentOffsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(pipeline->impl.textureBindings, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	parse_shader((uint32_t *)pipeline->vertexShader->impl.source, pipeline->vertexShader->impl.length, pipeline->impl.vertexLocations,
	             pipeline->impl.textureBindings, pipeline->impl.vertexOffsets);
	parse_shader((uint32_t *)pipeline->fragmentShader->impl.source, pipeline->fragmentShader->impl.length, pipeline->impl.fragmentLocations,
	             pipeline->impl.textureBindings, pipeline->impl.fragmentOffsets);*/

	const VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .pNext = NULL,
	    .setLayoutCount = 1,
	    .pSetLayouts = &desc_layout,
	};

	VkResult result = vkCreatePipelineLayout(device->device, &pipeline_layout_create_info, NULL, &pipe->pipeline_layout);
	assert(result == VK_SUCCESS);

	VkPipelineInputAssemblyStateCreateInfo ia = {0};
	VkPipelineRasterizationStateCreateInfo rs = {0};
	VkPipelineColorBlendStateCreateInfo cb = {0};
	VkPipelineDepthStencilStateCreateInfo ds = {0};
	VkPipelineViewportStateCreateInfo vp = {0};
	VkPipelineMultisampleStateCreateInfo ms = {0};
#define dynamicStatesCount 2
	VkDynamicState dynamicStateEnables[dynamicStatesCount];
	VkPipelineDynamicStateCreateInfo dynamicState = {0};

	memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
	memset(&dynamicState, 0, sizeof(dynamicState));
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables;

	VkGraphicsPipelineCreateInfo pipeline_create_info = {
	    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    .layout = pipe->pipeline_layout,
	    .renderPass = VK_NULL_HANDLE,
	};

	int vertexAttributeCount = 0;
	int vertexBindingCount = 0;
	for (int i = 0; i < 16; ++i) {
		if (pipeline->inputLayout[i] == NULL) {
			break;
		}
		vertexAttributeCount += pipeline->inputLayout[i]->size;
		vertexBindingCount++;
	}

#ifdef KINC_WINDOWS
	VkVertexInputBindingDescription *vi_bindings = (VkVertexInputBindingDescription *)alloca(sizeof(VkVertexInputBindingDescription) * vertexBindingCount);
#else
	VkVertexInputBindingDescription vi_bindings[vertexBindingCount];
#endif
#ifdef KINC_WINDOWS
	VkVertexInputAttributeDescription *vi_attrs = (VkVertexInputAttributeDescription *)alloca(sizeof(VkVertexInputAttributeDescription) * vertexAttributeCount);
#else
	VkVertexInputAttributeDescription vi_attrs[vertexAttributeCount];
#endif
	VkPipelineVertexInputStateCreateInfo vi = {0};
	memset(&vi, 0, sizeof(vi));
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;
	vi.vertexBindingDescriptionCount = vertexBindingCount;
	vi.pVertexBindingDescriptions = vi_bindings;
	vi.vertexAttributeDescriptionCount = vertexAttributeCount;
	vi.pVertexAttributeDescriptions = vi_attrs;

	uint32_t attr = 0;
	for (int binding = 0; binding < vertexBindingCount; ++binding) {
		uint32_t offset = 0;
		uint32_t stride = 0;
		for (int i = 0; i < pipeline->inputLayout[binding]->size; ++i) {
			kinc_g5_vertex_element_t element = pipeline->inputLayout[binding]->elements[i];

			vi_attrs[attr].binding = binding;
			vi_attrs[attr].location = find_number(pipeline->impl.vertexLocations, element.name);
			vi_attrs[attr].offset = offset;
			offset += kinc_g4_vertex_data_size(element.data);
			stride += kinc_g4_vertex_data_size(element.data);

			switch (element.data) {
			case KINC_G4_VERTEX_DATA_F32_1X:
				vi_attrs[attr].format = VK_FORMAT_R32_SFLOAT;
				break;
			case KINC_G4_VERTEX_DATA_F32_2X:
				vi_attrs[attr].format = VK_FORMAT_R32G32_SFLOAT;
				break;
			case KINC_G4_VERTEX_DATA_F32_3X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case KINC_G4_VERTEX_DATA_F32_4X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			case KINC_G4_VERTEX_DATA_I8_1X:
				vi_attrs[attr].format = VK_FORMAT_R8_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X:
				vi_attrs[attr].format = VK_FORMAT_R8_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X:
				vi_attrs[attr].format = VK_FORMAT_R8G8_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X:
				vi_attrs[attr].format = VK_FORMAT_R8G8_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8G8_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8G8_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X:
				vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X:
				vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X:
				vi_attrs[attr].format = VK_FORMAT_R16_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X:
				vi_attrs[attr].format = VK_FORMAT_R16_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X:
				vi_attrs[attr].format = VK_FORMAT_R16G16_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X:
				vi_attrs[attr].format = VK_FORMAT_R16G16_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16G16_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16G16_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X:
				vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X:
				vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_SNORM;
				break;
			case KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED:
				vi_attrs[attr].format = VK_FORMAT_R16G16B16A16_UNORM;
				break;
			case KINC_G4_VERTEX_DATA_I32_1X:
				vi_attrs[attr].format = VK_FORMAT_R32_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U32_1X:
				vi_attrs[attr].format = VK_FORMAT_R32_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I32_2X:
				vi_attrs[attr].format = VK_FORMAT_R32G32_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U32_2X:
				vi_attrs[attr].format = VK_FORMAT_R32G32_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I32_3X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U32_3X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32_UINT;
				break;
			case KINC_G4_VERTEX_DATA_I32_4X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_SINT;
				break;
			case KINC_G4_VERTEX_DATA_U32_4X:
				vi_attrs[attr].format = VK_FORMAT_R32G32B32A32_UINT;
				break;
			default:
				assert(false);
				break;
			}
			attr++;
		}
		vi_bindings[binding].binding = binding;
		vi_bindings[binding].stride = stride;
		vi_bindings[binding].inputRate = pipeline->inputLayout[binding]->instanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
	}

	memset(&ia, 0, sizeof(ia));
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	memset(&rs, 0, sizeof(rs));
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = convert_cull_mode(pipeline->cullMode);
	rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.depthClampEnable = VK_FALSE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = VK_FALSE;
	rs.lineWidth = 1.0f;

	memset(&cb, 0, sizeof(cb));
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	VkPipelineColorBlendAttachmentState att_state[8];
	memset(att_state, 0, sizeof(att_state));
	for (int i = 0; i < pipeline->colorAttachmentCount; ++i) {
		att_state[i].colorWriteMask =
		    (pipeline->colorWriteMaskRed[i] ? VK_COLOR_COMPONENT_R_BIT : 0) | (pipeline->colorWriteMaskGreen[i] ? VK_COLOR_COMPONENT_G_BIT : 0) |
		    (pipeline->colorWriteMaskBlue[i] ? VK_COLOR_COMPONENT_B_BIT : 0) | (pipeline->colorWriteMaskAlpha[i] ? VK_COLOR_COMPONENT_A_BIT : 0);
		att_state[i].blendEnable = pipeline->blend_source != KINC_G5_BLEND_ONE || pipeline->blend_destination != KINC_G5_BLEND_ZERO ||
		                           pipeline->alpha_blend_source != KINC_G5_BLEND_ONE || pipeline->alpha_blend_destination != KINC_G5_BLEND_ZERO;
		att_state[i].srcColorBlendFactor = convert_blend_factor(pipeline->blend_source);
		att_state[i].dstColorBlendFactor = convert_blend_factor(pipeline->blend_destination);
		att_state[i].colorBlendOp = convert_blend_operation(pipeline->blend_operation);
		att_state[i].srcAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_source);
		att_state[i].dstAlphaBlendFactor = convert_blend_factor(pipeline->alpha_blend_destination);
		att_state[i].alphaBlendOp = convert_blend_operation(pipeline->alpha_blend_operation);
	}
	cb.attachmentCount = pipeline->colorAttachmentCount;
	cb.pAttachments = att_state;

	memset(&vp, 0, sizeof(vp));
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	vp.scissorCount = 1;
	dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	memset(&ds, 0, sizeof(ds));
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = pipeline->depthMode != KINC_G5_COMPARE_MODE_ALWAYS;
	ds.depthWriteEnable = pipeline->depthWrite;
	ds.depthCompareOp = convert_compare_mode(pipeline->depthMode);
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.back.failOp = VK_STENCIL_OP_KEEP;
	ds.back.passOp = VK_STENCIL_OP_KEEP;
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = ds.back;

	memset(&ms, 0, sizeof(ms));
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pSampleMask = NULL;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	pipeline_create_info.stageCount = 2;
	VkPipelineShaderStageCreateInfo shaderStages[2];
	memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = prepare_vs(&pipeline->impl.vert_shader_module, pipeline->vertexShader);
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = prepare_fs(&pipeline->impl.frag_shader_module, pipeline->fragmentShader);
	shaderStages[1].pName = "main";

	pipeline_create_info.pVertexInputState = &vi;
	pipeline_create_info.pInputAssemblyState = &ia;
	pipeline_create_info.pRasterizationState = &rs;
	pipeline_create_info.pColorBlendState = &cb;
	pipeline_create_info.pMultisampleState = &ms;
	pipeline_create_info.pViewportState = &vp;
	pipeline_create_info.pDepthStencilState = &ds;
	pipeline_create_info.pStages = shaderStages;
	pipeline_create_info.pDynamicState = &dynamicState;

	result = vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline->pipeline);
	assert(result == VK_SUCCESS);

	vkDestroyShaderModule(device->device, pipeline->impl.frag_shader_module, NULL);
	vkDestroyShaderModule(device->device, pipeline->impl.vert_shader_module, NULL);
}

void kope_vulkan_render_pipeline_destroy(kope_vulkan_render_pipeline *pipe) {}

void kope_vulkan_compute_pipeline_init(kope_vulkan_device *device, kope_vulkan_compute_pipeline *pipe,
                                       const kope_vulkan_compute_pipeline_parameters *parameters) {}

void kope_vulkan_compute_pipeline_destroy(kope_vulkan_compute_pipeline *pipe) {}

void kope_vulkan_ray_pipeline_init(kope_g5_device *device, kope_vulkan_ray_pipeline *pipe, const kope_vulkan_ray_pipeline_parameters *parameters) {}

void kope_vulkan_ray_pipeline_destroy(kope_vulkan_ray_pipeline *pipe) {}
