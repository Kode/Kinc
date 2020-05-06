#include "pch.h"

#include "PipelineState.h"
#include <Kore/Graphics5/Graphics.h>

using namespace Kore;

Graphics5::PipelineState::PipelineState() {
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

	colorAttachmentCount = 1;
	for (int i = 0; i < 8; ++i) colorAttachment[i] = Target32Bit;
}

Graphics5::PipelineState::~PipelineState() {}
