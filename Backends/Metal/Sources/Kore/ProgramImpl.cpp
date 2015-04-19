#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace Kore;

ProgramImpl::ProgramImpl() {

}

Program::Program() {
	
}
	
void Program::setVertexShader(Shader* vertexShader) {
	this->vertexShader = vertexShader;
}
	
void Program::setFragmentShader(Shader* fragmentShader) {
	this->fragmentShader = fragmentShader;
}

void Program::link(const VertexStructure& structure) {

}

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
