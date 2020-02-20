#include "pch.h"

#include <string.h>
#include <kinc/graphics5/shader.h>

extern WGPUDevice device;

void kinc_g5_shader_init(kinc_g5_shader_t *shader, void *source, size_t length, kinc_g5_shader_type_t type) {
	WGPUShaderModuleDescriptor smDesc;
	memset(&smDesc, 0, sizeof(smDesc));
	smDesc.codeSize = length / 4;
	smDesc.code = source;
	shader->impl.module = wgpuDeviceCreateShaderModule(device, &smDesc);
}
