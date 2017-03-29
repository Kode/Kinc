#include "pch.h"

#include "ProgramImpl.h"

#include <Kore/Graphics/Shader.h>

using namespace Kore;

Kore::ProgramImpl::ProgramImpl() {}

Program::Program() {}

void Program::set() {

}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	return unit;
}

void Program::setVertexShader(Shader* shader) {

}

void Program::setFragmentShader(Shader* shader) {

}

void Program::setGeometryShader(Shader* shader) {

}

void Program::setTessellationControlShader(Shader* shader) {

}

void Program::setTessellationEvaluationShader(Shader* shader) {

}

void Program::link(VertexStructure** structures, int count) {

}
