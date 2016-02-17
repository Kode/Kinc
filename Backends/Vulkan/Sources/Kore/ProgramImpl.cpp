#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace Kore;

namespace Kore {
#ifndef OPENGLES
	bool programUsesTesselation = false;
#endif
}

ProgramImpl::ProgramImpl() : textureCount(0), vertexShader(nullptr), fragmentShader(nullptr), geometryShader(nullptr), tesselationEvaluationShader(nullptr), tesselationControlShader(nullptr) {
	textures = new const char*[16];
	textureValues = new int[16];
}

Program::Program() {

}

ProgramImpl::~ProgramImpl() {

}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

void Program::setGeometryShader(Shader* shader) {
	geometryShader = shader;
}

void Program::setTesselationControlShader(Shader* shader) {
	tesselationControlShader = shader;
}

void Program::setTesselationEvaluationShader(Shader* shader) {
	tesselationEvaluationShader = shader;
}

void Program::link(VertexStructure** structures, int count) {

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
