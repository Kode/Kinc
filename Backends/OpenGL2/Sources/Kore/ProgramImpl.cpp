#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include "ogl.h"
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
	programId = glCreateProgram();
	glCheckErrors();
}

ProgramImpl::~ProgramImpl() {
	glDeleteProgram(programId);
}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

void Program::setGeometryShader(Shader* shader) {
#ifndef OPENGLES
	geometryShader = shader;
#endif
}

void Program::setTesselationControlShader(Shader* shader) {
#ifndef OPENGLES
	tesselationControlShader = shader;
#endif
}

void Program::setTesselationEvaluationShader(Shader* shader) {
#ifndef OPENGLES
	tesselationEvaluationShader = shader;
#endif
}

namespace {
	int toGlShader(ShaderType type) {
		switch (type) {
		case VertexShader:
		default:
			return GL_VERTEX_SHADER;
		case FragmentShader:
			return GL_FRAGMENT_SHADER;
#ifndef OPENGLES
		case GeometryShader:
			return GL_GEOMETRY_SHADER;
		case TesselationControlShader:
			return GL_TESS_CONTROL_SHADER;
		case TesselationEvaluationShader:
			return GL_TESS_EVALUATION_SHADER;
#endif
		}
	}

	void compileShader(uint& id, char* source, int length, ShaderType type) {
		id = glCreateShader(toGlShader(type));
		glCheckErrors();
		glShaderSource(id, 1, (const GLchar**)&source, 0);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result != GL_TRUE) {
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* errormessage = new char[length];
			glGetShaderInfoLog(id, length, nullptr, errormessage);
			printf("GLSL compiler error: %s\n", errormessage);
			delete[] errormessage;
		}
	}
}

void Program::link(VertexStructure** structures, int count) {
	compileShader(vertexShader->id, vertexShader->source, vertexShader->length, VertexShader);
	compileShader(fragmentShader->id, fragmentShader->source, fragmentShader->length, FragmentShader);
#ifndef OPENGLES
	if (geometryShader != nullptr) compileShader(geometryShader->id, geometryShader->source, geometryShader->length, GeometryShader);
	if (tesselationControlShader != nullptr) compileShader(tesselationControlShader->id, tesselationControlShader->source, tesselationControlShader->length, TesselationControlShader);
	if (tesselationEvaluationShader != nullptr) compileShader(tesselationEvaluationShader->id, tesselationEvaluationShader->source, tesselationEvaluationShader->length, TesselationEvaluationShader);
#endif
	glAttachShader(programId, vertexShader->id);
	glAttachShader(programId, fragmentShader->id);
#ifndef OPENGLES
	if (geometryShader != nullptr) glAttachShader(programId, geometryShader->id);
	if (tesselationControlShader != nullptr) glAttachShader(programId, tesselationControlShader->id);
	if (tesselationEvaluationShader != nullptr) glAttachShader(programId, tesselationEvaluationShader->id);
#endif
	glCheckErrors();

	int index = 0;
	for (int i1 = 0; i1 < count; ++i1) {
		for (int i2 = 0; i2 < structures[i1]->size; ++i2) {
			VertexElement element = structures[i1]->elements[i2];
			glBindAttribLocation(programId, index, element.name);
			glCheckErrors();
			if (element.data == Float4x4VertexData) {
				index += 4;
			}
			else {
				++index;
			}
		}
	}

	glLinkProgram(programId);

	int result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = new char[length];
		glGetProgramInfoLog(programId, length, nullptr, errormessage);
		printf("GLSL linker error: %s\n", errormessage);
		delete[] errormessage;
	}

#ifndef OPENGLES
#ifndef SYS_LINUX
	if (tesselationControlShader != nullptr) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glCheckErrors();
	}
#endif
#endif
}

void Program::set() {
#ifndef OPENGLES
	programUsesTesselation = tesselationControlShader != nullptr;
#endif
	glUseProgram(programId);
	glCheckErrors();
	for (int index = 0; index < textureCount; ++index) {
		glUniform1i(textureValues[index], index);
		glCheckErrors();
	}
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.location = glGetUniformLocation(programId, name);
	glCheckErrors();
	if (location.location < 0) {
		log(Warning, "Uniform %s not found.", name);
	}
	return location;
}

int ProgramImpl::findTexture(const char* name) {
	for (int index = 0; index < textureCount; ++index) {
		if (strcmp(textures[index], name) == 0) return index;
	}
	return -1;
}

TextureUnit Program::getTextureUnit(const char* name) {
	int index = findTexture(name);
	if (index < 0) {
		int location = glGetUniformLocation(programId, name);
		glCheckErrors();
		index = textureCount;
		textureValues[index] = location;
		textures[index] = name;
		++textureCount;
	}
	TextureUnit unit;
	unit.unit = index;
	return unit;
}
