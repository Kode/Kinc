#include "PipelineState.h"

#include <Kore/Graphics5/Graphics.h>

#include <kinc/graphics5/vertexstructure.h>

using namespace Kore::Graphics5;

PipelineState::PipelineState() {
	for (int i = 0; i < 16; ++i) {
		inputLayout[i] = nullptr;
	}
	vertexShader = nullptr;
	fragmentShader = nullptr;
	geometryShader = nullptr;
	tessellationControlShader = nullptr;
	tessellationEvaluationShader = nullptr;

	cullMode = NoCulling;

	depthWrite = false;
	depthMode = ZCompareAlways;

	stencilMode = ZCompareAlways;
	stencilBothPass = Keep;
	stencilDepthFail = Keep;
	stencilFail = Keep;
	stencilReferenceValue = 0;
	stencilReadMask = 0xff;
	stencilWriteMask = 0xff;

	blendSource = BlendOne;
	blendDestination = BlendZero;
	// blendOperation = BlendingOperation.Add;
	alphaBlendSource = BlendOne;
	alphaBlendDestination = BlendZero;
	// alphaBlendOperation = BlendingOperation.Add;

	for (int i = 0; i < 8; ++i) {
		colorWriteMaskRed[i] = true;
		colorWriteMaskGreen[i] = true;
		colorWriteMaskBlue[i] = true;
		colorWriteMaskAlpha[i] = true;
	}

	colorAttachmentCount = 1;
	for (int i = 0; i < 8; ++i) {
		colorAttachment[i] = Target32Bit;
	}

	depthAttachmentBits = 0;
	stencilAttachmentBits = 0;

	kinc_g5_pipeline_init(&kincPipeline);
}

PipelineState::~PipelineState() {
	kinc_g5_pipeline_destroy(&kincPipeline);
}

void Kore_Internal_ConvertVertexStructure(kinc_g4_vertex_structure_t *target, const Kore::Graphics4::VertexStructure *source);

void PipelineState::compile() {
	kinc_g4_vertex_structure_t inputLayout[16];
	for (int i = 0; i < 16; ++i) {
		if (this->inputLayout[i] != nullptr) {
			kinc_g4_vertex_structure_init(&inputLayout[i]);
			Kore_Internal_ConvertVertexStructure(&inputLayout[i], this->inputLayout[i]);
			kincPipeline.inputLayout[i] = &inputLayout[i];
		}
		else {
			kincPipeline.inputLayout[i] = nullptr;
			break;
		}
	}

	kincPipeline.vertexShader = &vertexShader->kincShader;
	kincPipeline.fragmentShader = &fragmentShader->kincShader;
	kincPipeline.geometryShader = geometryShader == nullptr ? nullptr : &geometryShader->kincShader;
	kincPipeline.tessellationControlShader = tessellationControlShader == nullptr ? nullptr : &tessellationControlShader->kincShader;
	kincPipeline.tessellationEvaluationShader = tessellationEvaluationShader == nullptr ? nullptr : &tessellationEvaluationShader->kincShader;

	kincPipeline.cullMode = (kinc_g5_cull_mode_t)cullMode;

	kincPipeline.depthWrite = depthWrite;
	kincPipeline.depthMode = (kinc_g5_compare_mode_t)depthMode;

	kincPipeline.stencilMode = (kinc_g5_compare_mode_t)stencilMode;
	kincPipeline.stencilBothPass = (kinc_g5_stencil_action_t)stencilBothPass;
	kincPipeline.stencilDepthFail = (kinc_g5_stencil_action_t)stencilDepthFail;
	kincPipeline.stencilFail = (kinc_g5_stencil_action_t)stencilFail;
	kincPipeline.stencilReferenceValue = stencilReferenceValue;
	kincPipeline.stencilReadMask = stencilReadMask;
	kincPipeline.stencilWriteMask = stencilWriteMask;

	kincPipeline.blendSource = (kinc_g5_blending_operation_t)blendSource;
	kincPipeline.blendDestination = (kinc_g5_blending_operation_t)blendDestination;
	// blendOperation = BlendingOperation.Add;
	kincPipeline.alphaBlendSource = (kinc_g5_blending_operation_t)alphaBlendSource;
	kincPipeline.alphaBlendDestination = (kinc_g5_blending_operation_t)alphaBlendDestination;
	// alphaBlendOperation = BlendingOperation.Add;

	for (int i = 0; i < 8; ++i) {
		kincPipeline.colorWriteMaskRed[i] = colorWriteMaskRed[i];
		kincPipeline.colorWriteMaskGreen[i] = colorWriteMaskGreen[i];
		kincPipeline.colorWriteMaskBlue[i] = colorWriteMaskBlue[i];
		kincPipeline.colorWriteMaskAlpha[i] = colorWriteMaskAlpha[i];
	}

	kincPipeline.colorAttachmentCount = 1;
	for (int i = 0; i < 8; ++i) {
		kincPipeline.colorAttachment[i] = (kinc_g5_render_target_format_t)colorAttachment[i];
	}

	kincPipeline.depthAttachmentBits = depthAttachmentBits;
	kincPipeline.stencilAttachmentBits = stencilAttachmentBits;

	kinc_g5_pipeline_compile(&kincPipeline);
}

ConstantLocation PipelineState::getConstantLocation(const char *name) {
	ConstantLocation location;
	location.kincConstantLocation = kinc_g5_pipeline_get_constant_location(&kincPipeline, name);
	return location;
}

TextureUnit PipelineState::getTextureUnit(const char *name) {
	TextureUnit unit;
	unit.kincTextureUnit = kinc_g5_pipeline_get_texture_unit(&kincPipeline, name);
	return unit;
}
