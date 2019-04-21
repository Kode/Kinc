#include "pch.h"

#include "PipelineState.h"
#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

Graphics4::PipelineState::PipelineState() {
	Kinc_G4_PipelineState_Create(&kincPipeline);

	for (int i = 0; i < 16; ++i) inputLayout[i] = nullptr;
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

	for (int i = 0; i < 8; ++i) colorWriteMaskRed[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskGreen[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskBlue[i] = true;
	for (int i = 0; i < 8; ++i) colorWriteMaskAlpha[i] = true;

	conservativeRasterization = false;
}

Graphics4::PipelineState::~PipelineState() {}

void Kore_Internal_ConvertVertexStructure(Kinc_G4_VertexStructure *target, const Graphics4::VertexStructure *source);

void Graphics4::PipelineState::compile() {
	Kinc_G4_VertexStructure inputLayout[16];
	for (int i = 0; i < 16; ++i) {
		if (this->inputLayout[i] != nullptr) {
			Kinc_G4_VertexStructure_Create(&inputLayout[i]);
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

	kincPipeline.cullMode = (Kinc_G4_CullMode)cullMode;

	kincPipeline.depthWrite = depthWrite;
	kincPipeline.depthMode = (Kinc_G4_CompareMode)depthMode;

	kincPipeline.stencilMode = (Kinc_G4_CompareMode)stencilMode;
	kincPipeline.stencilBothPass = (Kinc_G4_StencilAction)stencilBothPass;
	kincPipeline.stencilDepthFail = (Kinc_G4_StencilAction)stencilDepthFail;
	kincPipeline.stencilFail = (Kinc_G4_StencilAction)stencilFail;
	kincPipeline.stencilReferenceValue = stencilReferenceValue;
	kincPipeline.stencilReadMask = stencilReadMask;
	kincPipeline.stencilWriteMask = stencilWriteMask;

	kincPipeline.blendSource = (Kinc_G4_BlendingOperation)blendSource;
	kincPipeline.blendDestination = (Kinc_G4_BlendingOperation)blendDestination;
	kincPipeline.alphaBlendSource = (Kinc_G4_BlendingOperation)alphaBlendSource;
	kincPipeline.alphaBlendDestination = (Kinc_G4_BlendingOperation)alphaBlendDestination;

	for (int i = 0; i < 8; ++i) {
		kincPipeline.colorWriteMaskRed[i] = colorWriteMaskRed[i];
		kincPipeline.colorWriteMaskGreen[i] = colorWriteMaskGreen[i];
		kincPipeline.colorWriteMaskBlue[i] = colorWriteMaskBlue[i];
		kincPipeline.colorWriteMaskAlpha[i] = colorWriteMaskAlpha[i];
	}

	kincPipeline.conservativeRasterization = conservativeRasterization;

	Kinc_G4_PipelineState_Compile(&kincPipeline);
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char *name) {
	Graphics4::ConstantLocation location;
	location.kincConstant = Kinc_G4_PipelineState_GetConstantLocation(&kincPipeline, name);
	return location;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char* name) {
	Graphics4::TextureUnit unit;
	unit.kincUnit = Kinc_G4_PipelineState_GetTextureUnit(&kincPipeline, name);
	return unit;
}
