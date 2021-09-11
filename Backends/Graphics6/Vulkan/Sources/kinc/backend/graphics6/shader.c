#include <kinc/graphics6/shader.h>
#include "vulkan.h"

void kinc_g6_shader_init(kinc_g6_shader_t *shader, const kinc_g6_shader_descriptor_t *descriptor) {
	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.codeSize = descriptor->code_size;
	create_info.pCode = descriptor->code;
	CHECK(vkCreateShaderModule(context.device, &create_info, NULL, &shader->impl.module));
}

void kinc_g6_shader_destroy(kinc_g6_shader_t *shader) {
	vkDestroyShaderModule(context.device, shader->impl.module, NULL);
}