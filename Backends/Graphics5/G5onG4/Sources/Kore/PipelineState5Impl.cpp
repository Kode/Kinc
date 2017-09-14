#include "pch.h"

#include "PipelineState5Impl.h"

#include <Kore/Graphics5/Shader.h>
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/WinError.h>

using namespace Kore;

PipelineState5Impl::PipelineState5Impl() {}

Graphics5::ConstantLocation Graphics5::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;



	return location;
}

Graphics5::TextureUnit Graphics5::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;



	return unit;
}

void Graphics5::PipelineState::compile() {

}
