#include "pch.h"

#include "PipelineState5Impl.h"

#include <Kore/Graphics4/PipelineState.h>

#include <Kore/Graphics5/Shader.h>
#include <Kore/Graphics5/PipelineState.h>

using namespace Kore;

PipelineState5Impl::PipelineState5Impl() {
	state = new Graphics4::PipelineState();
}

Graphics5::ConstantLocation Graphics5::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.location = new Graphics4::ConstantLocation(state->getConstantLocation(name));
	return location;
}

Graphics5::TextureUnit Graphics5::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit.unit = 0;
	return unit;
}

void Graphics5::PipelineState::compile() {
	state->inputLayout[0] = inputLayout[0];
	state->vertexShader = vertexShader->shader;
	state->fragmentShader = fragmentShader->shader;
	state->compile();
}
