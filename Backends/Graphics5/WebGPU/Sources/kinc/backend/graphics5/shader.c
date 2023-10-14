#include <kinc/graphics5/shader.h>

#include <string.h>

extern WGPUDevice device;

#ifdef KINC_KONG
WGPUShaderModule kinc_g5_internal_webgpu_shader_module;

void kinc_g5_internal_webgpu_create_shader_module(const void *source, size_t length) {
	WGPUShaderModuleWGSLDescriptor wgsl_desc = {0};
	wgsl_desc.code = (const char *)source;
	wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;

	WGPUShaderModuleDescriptor desc = {0};
	desc.nextInChain = (WGPUChainedStruct *)(&wgsl_desc);

	kinc_g5_internal_webgpu_shader_module = wgpuDeviceCreateShaderModule(device, &desc);
}

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	strcpy(&shader->impl.entry_name[0], source);
}
#else
void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	WGPUShaderModuleSPIRVDescriptor smSpirvDesc;
	memset(&smSpirvDesc, 0, sizeof(smSpirvDesc));
	smSpirvDesc.chain.sType = WGPUSType_ShaderModuleSPIRVDescriptor;
	smSpirvDesc.codeSize = length / 4;
	smSpirvDesc.code = source;
	WGPUShaderModuleDescriptor smDesc;
	memset(&smDesc, 0, sizeof(smDesc));
	smDesc.nextInChain = &smSpirvDesc;
	shader->impl.module = wgpuDeviceCreateShaderModule(device, &smDesc);
}
#endif

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {}
