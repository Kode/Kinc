#include "PipelineState.h"
#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

Graphics4::PipelineState::PipelineState() {
	kinc_g4_pipeline_init(&kincPipeline);

	for (int i = 0; i < 16; ++i) inputLayout[i] = nullptr;
	vertexShader = nullptr;
	fragmentShader = nullptr;
	geometryShader = nullptr;
	tessellationControlShader = nullptr;
	tessellationEvaluationShader = nullptr;

	cullMode = NoCulling;

	depthWrite = false;
	depthMode = ZCompareAlways;

	stencilFrontMode = ZCompareAlways;
	stencilFrontBothPass = Keep;
	stencilFrontDepthFail = Keep;
	stencilFrontFail = Keep;

	stencilBackMode = ZCompareAlways;
	stencilBackBothPass = Keep;
	stencilBackDepthFail = Keep;
	stencilBackFail = Keep;

	stencilReferenceValue = 0;
	stencilReadMask = 0xff;
	stencilWriteMask = 0xff;

	blendSource = BlendOne;
	blendDestination = BlendZero;
	blendOperation = BlendOpAdd;
	alphaBlendSource = BlendOne;
	alphaBlendDestination = BlendZero;
	alphaBlendOperation = BlendOpAdd;

	for (int i = 0; i < 8; ++i) colorWriteMaskRed[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskGreen[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskBlue[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskAlpha[i] = true;

	colorAttachmentCount = 1;
	for (int i = 0; i < 8; ++i) colorAttachment[i] = Target32Bit;
	depthAttachmentBits = 0;
	stencilAttachmentBits = 0;

	conservativeRasterization = false;
}

Graphics4::PipelineState::~PipelineState() {
	kinc_g4_pipeline_destroy(&kincPipeline);
}

void Kore_Internal_ConvertVertexStructure(kinc_g4_vertex_structure_t *target, const Graphics4::VertexStructure *source);

void Graphics4::PipelineState::compile() {
	kinc_g4_vertex_structure_t inputLayout[16];
	for (int i = 0; i < 16; ++i) {
		if (this->inputLayout[i] != nullptr) {
			kinc_g4_vertex_structure_init(&inputLayout[i]);
			Kore_Internal_ConvertVertexStructure(&inputLayout[i], this->inputLayout[i]);
			kincPipeline.input_layout[i] = &inputLayout[i];
		}
		else {
			kincPipeline.input_layout[i] = nullptr;
			break;
		}
	}

	kincPipeline.vertex_shader = &vertexShader->kincShader;
	kincPipeline.fragment_shader = &fragmentShader->kincShader;
	kincPipeline.geometry_shader = geometryShader == nullptr ? nullptr : &geometryShader->kincShader;
	kincPipeline.tessellation_control_shader = tessellationControlShader == nullptr ? nullptr : &tessellationControlShader->kincShader;
	kincPipeline.tessellation_evaluation_shader = tessellationEvaluationShader == nullptr ? nullptr : &tessellationEvaluationShader->kincShader;

	kincPipeline.cull_mode = (kinc_g4_cull_mode_t)cullMode;

	kincPipeline.depth_write = depthWrite;
	kincPipeline.depth_mode = (kinc_g4_compare_mode_t)depthMode;

	kincPipeline.stencil_front_mode = (kinc_g4_compare_mode_t)stencilFrontMode;
	kincPipeline.stencil_front_both_pass = (kinc_g4_stencil_action_t)stencilFrontBothPass;
	kincPipeline.stencil_front_depth_fail = (kinc_g4_stencil_action_t)stencilFrontDepthFail;
	kincPipeline.stencil_front_fail = (kinc_g4_stencil_action_t)stencilFrontFail;

	kincPipeline.stencil_back_mode = (kinc_g4_compare_mode_t)stencilBackMode;
	kincPipeline.stencil_back_both_pass = (kinc_g4_stencil_action_t)stencilBackBothPass;
	kincPipeline.stencil_back_depth_fail = (kinc_g4_stencil_action_t)stencilBackDepthFail;
	kincPipeline.stencil_back_fail = (kinc_g4_stencil_action_t)stencilBackFail;

	kincPipeline.stencil_reference_value = stencilReferenceValue;
	kincPipeline.stencil_read_mask = stencilReadMask;
	kincPipeline.stencil_write_mask = stencilWriteMask;

	kincPipeline.blend_source = (kinc_g4_blending_factor_t)blendSource;
	kincPipeline.blend_destination = (kinc_g4_blending_factor_t)blendDestination;
	kincPipeline.blend_operation = (kinc_g4_blending_operation_t)blendOperation;
	kincPipeline.alpha_blend_source = (kinc_g4_blending_factor_t)alphaBlendSource;
	kincPipeline.alpha_blend_destination = (kinc_g4_blending_factor_t)alphaBlendDestination;
	kincPipeline.alpha_blend_operation = (kinc_g4_blending_operation_t)alphaBlendOperation;

	kincPipeline.color_attachment_count = colorAttachmentCount;

	for (int i = 0; i < 8; ++i) {
		kincPipeline.color_write_mask_red[i] = colorWriteMaskRed[i];
		kincPipeline.color_write_mask_green[i] = colorWriteMaskGreen[i];
		kincPipeline.color_write_mask_blue[i] = colorWriteMaskBlue[i];
		kincPipeline.color_write_mask_alpha[i] = colorWriteMaskAlpha[i];
		kincPipeline.color_attachment[i] = (kinc_g4_render_target_format_t)colorAttachment[i];
	}

	kincPipeline.depth_attachment_bits = depthAttachmentBits;
	kincPipeline.stencil_attachment_bits = stencilAttachmentBits;

	kincPipeline.conservative_rasterization = conservativeRasterization;

	kinc_g4_pipeline_compile(&kincPipeline);
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char *name) {
	Graphics4::ConstantLocation location;
	location.kincConstant = kinc_g4_pipeline_get_constant_location(&kincPipeline, name);
	return location;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char *name) {
	Graphics4::TextureUnit unit;
	unit.kincUnit = kinc_g4_pipeline_get_texture_unit(&kincPipeline, name);
	return unit;
}
