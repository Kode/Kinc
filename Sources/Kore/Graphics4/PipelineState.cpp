#include "pch.h"

#include "PipelineState.h"
#include <Kore/Graphics4/Graphics.h>

using namespace Kore;

/*
Graphics4::PipelineState::PipelineState() : Program() {
	// TODO
	// program = new Program;
}

void Graphics4::PipelineState::compile() {
	link(PipelineStateBase::inputLayout, 0);
}

//ConstantLocation PipelineState::getConstantLocation(const char* name) {
//    return new ConstantLocation(glGetUniformLocation(program, name));
//}
*/

Graphics4::PipelineState::PipelineState() {
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
	//blendOperation = BlendingOperation.Add;
	alphaBlendSource = BlendOne;
	alphaBlendDestination = BlendZero;
	//alphaBlendOperation = BlendingOperation.Add;

	colorWriteMaskRed = true;
	colorWriteMaskGreen = true;
	colorWriteMaskBlue = true;
	colorWriteMaskAlpha = true;
}

Graphics4::PipelineState::~PipelineState() {

}
