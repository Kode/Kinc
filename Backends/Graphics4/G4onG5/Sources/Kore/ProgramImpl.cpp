#include "pch.h"

#include "ProgramImpl.h"

#include <Kore/Graphics4/Shader.h>

using namespace Kore;

Kore::ProgramImpl::ProgramImpl() {}

Graphics4::Program::Program() {}

void Graphics4::Program::set() {
	_program.set();
}

Graphics4::ConstantLocation Graphics4::Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	location._location = _program.getConstantLocation(name);
	return location;
}

Graphics4::TextureUnit Graphics4::Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit._unit = _program.getTextureUnit(name);
	return unit;
}

void Graphics4::Program::setVertexShader(Shader* shader) {
	_program.setVertexShader(&shader->_shader);
}

void Graphics4::Program::setFragmentShader(Shader* shader) {
	_program.setFragmentShader(&shader->_shader);
}

void Graphics4::Program::setGeometryShader(Shader* shader) {
	_program.setGeometryShader(&shader->_shader);
}

void Graphics4::Program::setTessellationControlShader(Shader* shader) {
	_program.setTessellationControlShader(&shader->_shader);
}

void Graphics4::Program::setTessellationEvaluationShader(Shader* shader) {
	_program.setTessellationEvaluationShader(&shader->_shader);
}

void Graphics4::Program::link(VertexStructure** structures, int count) {
	_program.link(structures, count);
}
