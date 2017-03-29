#include "pch.h"

#include "ProgramImpl.h"

#include <Kore/Graphics/Shader.h>

using namespace Kore;

Kore::ProgramImpl::ProgramImpl() {}

Program::Program() {}

void Program::set() {
	_program.set();
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	location._location = _program.getConstantLocation(name);
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit._unit = _program.getTextureUnit(name);
	return unit;
}

void Program::setVertexShader(Shader* shader) {
	_program.setVertexShader(&shader->_shader);
}

void Program::setFragmentShader(Shader* shader) {
	_program.setFragmentShader(&shader->_shader);
}

void Program::setGeometryShader(Shader* shader) {
	_program.setGeometryShader(&shader->_shader);
}

void Program::setTessellationControlShader(Shader* shader) {
	_program.setTessellationControlShader(&shader->_shader);
}

void Program::setTessellationEvaluationShader(Shader* shader) {
	_program.setTessellationEvaluationShader(&shader->_shader);
}

void Program::link(VertexStructure** structures, int count) {
	_program.link(structures, count);
}
