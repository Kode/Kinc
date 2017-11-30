#include "pch.h"

#include "PipelineStateImpl.h"
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics5/PipelineState.h>

#include <Kore/Graphics4/Shader.h>

using namespace Kore;

Kore::PipelineStateImpl::PipelineStateImpl() {
	_pipeline = new Graphics5::PipelineState();
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;
	location._location = _pipeline->getConstantLocation(name);
	return location;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit._unit = _pipeline->getTextureUnit(name);
	return unit;
}

void Graphics4::PipelineState::compile() {
	for (int i = 0; i < 16; ++i) {
		_pipeline->inputLayout[i] = inputLayout[i];
	}
	_pipeline->vertexShader = &vertexShader->_shader;
	_pipeline->fragmentShader = &fragmentShader->_shader;
	_pipeline->geometryShader = geometryShader != nullptr ? &geometryShader->_shader : nullptr;
	_pipeline->tessellationControlShader = tessellationControlShader != nullptr ? &tessellationControlShader->_shader : nullptr;
	_pipeline->tessellationEvaluationShader = tessellationEvaluationShader != nullptr ? &tessellationEvaluationShader->_shader : nullptr;
	_pipeline->blendSource = (Graphics5::BlendingOperation)blendSource;
	_pipeline->blendDestination = (Graphics5::BlendingOperation)blendDestination;
	_pipeline->alphaBlendSource = (Graphics5::BlendingOperation)alphaBlendSource;
	_pipeline->alphaBlendDestination = (Graphics5::BlendingOperation)alphaBlendDestination;
	_pipeline->compile();
}
