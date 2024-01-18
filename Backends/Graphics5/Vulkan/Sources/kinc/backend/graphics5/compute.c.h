#include <kinc/graphics5/compute.h>

#include <kinc/graphics4/texture.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

static void parse_shader(uint32_t *shader_source, int shader_length, kinc_internal_named_number *locations, kinc_internal_named_number *textureBindings,
                        kinc_internal_named_number *uniformOffsets);

static uint8_t constantsMemory[1024 * 4];

static int getMultipleOf16(int value) {
	int ret = 16;
	while (ret < value)
		ret += 16;
	return ret;
}

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *_data, int length) {
	memset(shader->impl.locations, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(shader->impl.offsets, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	memset(shader->impl.texture_bindings, 0, sizeof(kinc_internal_named_number) * KINC_INTERNAL_NAMED_NUMBER_COUNT);
	parse_shader((uint32_t *)_data, length, shader->impl.locations, shader->impl.texture_bindings, shader->impl.offsets);

	/*shader->impl.length = (int)(length - index);
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	assert(shader->impl.data != NULL);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	VkShaderModuleCreateInfo module_create_info;
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.pNext = NULL;
	module_create_info.codeSize = shader->impl.length;
	module_create_info.pCode = (const uint32_t *)shader->impl.data;
	module_create_info.flags = 0;

	VkShaderModule module;
	VkResult err = vkCreateShaderModule(vk_ctx.device, &module_create_info, NULL, &module);
	assert(!err);

	VkPipelineShaderStageCreateInfo compute_shader_stage_info = {0};
	compute_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compute_shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compute_shader_stage_info.module = module;
	compute_shader_stage_info.pName = "main";

	VkDescriptorSetLayoutBinding layout_bindings[1] = {0};
	layout_bindings[0].binding = 0;
	layout_bindings[0].descriptorCount = 1;
	layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_bindings[0].pImmutableSamplers = NULL;
	layout_bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	/*layout_bindings[1].binding = 1;
	layout_bindings[1].descriptorCount = 1;
	layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layout_bindings[1].pImmutableSamplers = NULL;
	layout_bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	layout_bindings[2].binding = 2;
	layout_bindings[2].descriptorCount = 1;
	layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layout_bindings[2].pImmutableSamplers = NULL;
	layout_bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;*/

	/*VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = layout_bindings;

	VkDescriptorSetLayout descriptor_set_layout;
	if (vkCreateDescriptorSetLayout(vk_ctx.device, &layoutInfo, NULL, &descriptor_set_layout) != VK_SUCCESS) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descriptor_set_layout;

	if (vkCreatePipelineLayout(vk_ctx.device, &pipeline_layout_info, NULL, &shader->impl.pipeline_layout) != VK_SUCCESS) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

	VkComputePipelineCreateInfo pipeline_info = {0};
	memset(&pipeline_info, 0, sizeof(pipeline_info));
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.layout = shader->impl.pipeline_layout;
	pipeline_info.stage = compute_shader_stage_info;

	if (vkCreateComputePipelines(vk_ctx.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &shader->impl.pipeline) != VK_SUCCESS) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}*/
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location = {0};

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	/*kinc_compute_internal_shader_constant_t *constant = findComputeConstant(shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.computeOffset = 0;
	}
	else {
		location.impl.computeOffset = constant->offset;
	}

	if (location.impl.computeOffset == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}*/

	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63)
		len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}
	/*kinc_internal_hash_index_t *compute_unit = findComputeTextureUnit(shader->impl.textures, hash);
	if (compute_unit == NULL) {
#ifndef NDEBUG
		static int notFoundCount = 0;
		if (notFoundCount < 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Sampler %s not found.", unitName);
			++notFoundCount;
		}
		else if (notFoundCount == 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Giving up on sampler not found messages.", unitName);
			++notFoundCount;
		}
#endif
	}
	else {
		unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] = compute_unit->index + unitOffset;
	}*/
	return unit;
}
