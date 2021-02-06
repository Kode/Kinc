#include "pch.h"

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
	WGPUColorStateDescriptor csDesc;
	memset(&csDesc, 0, sizeof(csDesc));
	csDesc.format = WGPUTextureFormat_BGRA8Unorm;
	csDesc.writeMask = WGPUColorWriteMask_All;
	csDesc.colorBlend.operation = WGPUBlendOperation_Add;
	csDesc.colorBlend.srcFactor = WGPUBlendFactor_One;
	csDesc.colorBlend.dstFactor = WGPUBlendFactor_Zero;
	csDesc.alphaBlend.operation = WGPUBlendOperation_Add;
	csDesc.alphaBlend.srcFactor = WGPUBlendFactor_One;
	csDesc.alphaBlend.dstFactor = WGPUBlendFactor_Zero;

	WGPUPipelineLayoutDescriptor plDesc;
	memset(&plDesc, 0, sizeof(plDesc));
	plDesc.bindGroupLayoutCount = 0;
	plDesc.bindGroupLayouts = NULL;

	WGPUProgrammableStageDescriptor vertDesc;
	memset(&vertDesc, 0, sizeof(vertDesc));
	vertDesc.module = pipe->vertexShader->impl.module;
	vertDesc.entryPoint = "main";

	WGPUProgrammableStageDescriptor fragDesc;
	memset(&fragDesc, 0, sizeof(fragDesc));
	fragDesc.module = pipe->fragmentShader->impl.module;
	fragDesc.entryPoint = "main";

	WGPUVertexAttributeDescriptor vaDesc[8];
	memset(&vaDesc[0], 0, sizeof(vaDesc[0]) * 8);
	uint64_t offset = 0;
	for (int i = 0; i < pipe->inputLayout[0]->size; ++i) {
		vaDesc[i].shaderLocation = i;
		vaDesc[i].offset = offset;
		switch (pipe->inputLayout[0]->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			vaDesc[i].format = WGPUVertexFormat_Float;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			vaDesc[i].format = WGPUVertexFormat_Float2;
			offset += 8;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			vaDesc[i].format = WGPUVertexFormat_Float3;
			offset += 12;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			vaDesc[i].format = WGPUVertexFormat_Float4;
			offset += 16;
			break;
		case KINC_G4_VERTEX_DATA_COLOR:
			vaDesc[i].format = WGPUVertexFormat_UChar4Norm;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			vaDesc[i].format = WGPUVertexFormat_Short2Norm;
			offset += 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			vaDesc[i].format = WGPUVertexFormat_Short4Norm;
			offset += 8;
			break;
		}
	}

	WGPUVertexBufferLayoutDescriptor vbDesc;
	memset(&vbDesc, 0, sizeof(vbDesc));
	vbDesc.arrayStride = offset;
	vbDesc.attributeCount = pipe->inputLayout[0]->size;
	vbDesc.attributes = &vaDesc[0];

	WGPUVertexStateDescriptor vsDest;
	memset(&vsDest, 0, sizeof(vsDest));
	vsDest.indexFormat = WGPUIndexFormat_Uint32;
	vsDest.vertexBufferCount = 1;
	vsDest.vertexBuffers = &vbDesc;

	WGPURasterizationStateDescriptor rsDesc;
	memset(&rsDesc, 0, sizeof(rsDesc));
	rsDesc.frontFace = WGPUFrontFace_CW;
	rsDesc.cullMode = WGPUCullMode_None;

	WGPURenderPipelineDescriptor rpDesc;
	memset(&rpDesc, 0, sizeof(rpDesc));
	rpDesc.layout = wgpuDeviceCreatePipelineLayout(device, &plDesc);
	rpDesc.vertexStage = vertDesc;
	rpDesc.fragmentStage = &fragDesc;
	rpDesc.colorStateCount = 1;
	rpDesc.colorStates = &csDesc;
	rpDesc.primitiveTopology = WGPUPrimitiveTopology_TriangleList;
	rpDesc.vertexState = &vsDest;
	rpDesc.sampleCount = 1;
	rpDesc.rasterizationState = &rsDesc;
	rpDesc.sampleMask = 0xffffffff;
	pipe->impl.pipeline = wgpuDeviceCreateRenderPipeline(device, &rpDesc);
}
