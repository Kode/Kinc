#include <string.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/constantlocation.h>

extern WGPUDevice device;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	kinc_g5_internal_pipeline_init(pipe);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char* name) {
	kinc_g5_constant_location_t location;
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	return unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	WGPUColorTargetState csDesc;
	memset(&csDesc, 0, sizeof(csDesc));
	csDesc.format = WGPUTextureFormat_BGRA8Unorm;
	csDesc.writeMask = WGPUColorWriteMask_All;
	WGPUBlendState blend;
	memset(&blend, 0, sizeof(blend));
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = WGPUBlendFactor_One;
	blend.color.dstFactor = WGPUBlendFactor_Zero;
	blend.alpha.operation = WGPUBlendOperation_Add;
	blend.alpha.srcFactor = WGPUBlendFactor_One;
	blend.alpha.dstFactor = WGPUBlendFactor_Zero;
	csDesc.blend = &blend;

	WGPUPipelineLayoutDescriptor plDesc;
	memset(&plDesc, 0, sizeof(plDesc));
	plDesc.bindGroupLayoutCount = 0;
	plDesc.bindGroupLayouts = NULL;

	WGPUVertexAttribute vaDesc[8];
	memset(&vaDesc[0], 0, sizeof(vaDesc[0]) * 8);
	uint64_t offset = 0;
	for (int i = 0; i < pipe->inputLayout[0]->size; ++i) {
		vaDesc[i].shaderLocation = i;
		vaDesc[i].offset = offset;
		switch (pipe->inputLayout[0]->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			vaDesc[i].format = WGPUVertexFormat_Float32;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			vaDesc[i].format = WGPUVertexFormat_Float32x2;
			offset += 8;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			vaDesc[i].format = WGPUVertexFormat_Float32x3;
			offset += 12;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			vaDesc[i].format = WGPUVertexFormat_Float32x4;
			offset += 16;
			break;
		case KINC_G4_VERTEX_DATA_COLOR:
			vaDesc[i].format = WGPUVertexFormat_Unorm8x4;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x2;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x4;
			offset += 8;
			break;
		}
	}

	WGPUVertexBufferLayout vbDesc;
	memset(&vbDesc, 0, sizeof(vbDesc));
	vbDesc.arrayStride = offset;
	vbDesc.attributeCount = pipe->inputLayout[0]->size;
	vbDesc.attributes = &vaDesc[0];

	WGPUVertexState vsDest;
	memset(&vsDest, 0, sizeof(vsDest));
	vsDest.module = pipe->vertexShader->impl.module;
	vsDest.entryPoint = "main";
	vsDest.bufferCount = 1;
	vsDest.buffers = &vbDesc;

	WGPUFragmentState fragmentDest;
	memset(&fragmentDest, 0, sizeof(fragmentDest));
	fragmentDest.module = pipe->fragmentShader->impl.module;
	fragmentDest.entryPoint = "main";
	fragmentDest.targetCount = 1;
	fragmentDest.targets = &csDesc;

	WGPUPrimitiveState rsDesc;
	memset(&rsDesc, 0, sizeof(rsDesc));
	rsDesc.topology = WGPUPrimitiveTopology_TriangleList;
	rsDesc.stripIndexFormat = WGPUIndexFormat_Uint32;
	rsDesc.frontFace = WGPUFrontFace_CW;
	rsDesc.cullMode = WGPUCullMode_None;

	WGPUMultisampleState multisample;
	memset(&multisample, 0, sizeof(multisample));
	multisample.count = 1;
	multisample.mask = 0xffffffff;
	multisample.alphaToCoverageEnabled = false;

	WGPURenderPipelineDescriptor2 rpDesc;
	memset(&rpDesc, 0, sizeof(rpDesc));
	rpDesc.layout = wgpuDeviceCreatePipelineLayout(device, &plDesc);
	rpDesc.fragment = &fragmentDest;
	rpDesc.vertex = vsDest;
	rpDesc.multisample = multisample;
	rpDesc.primitive = rsDesc;
	pipe->impl.pipeline = wgpuDeviceCreateRenderPipeline2(device, &rpDesc);
}
