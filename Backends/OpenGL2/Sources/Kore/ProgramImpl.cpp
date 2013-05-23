#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include "ogl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace Kore;

ProgramImpl::ProgramImpl() : textureCount(0) {
	textures = new const char*[16];
	textureValues = new int[16];
}

Program::Program() {
	programId = glCreateProgram();
}
	
void Program::setVertexShader(Shader* vertexShader) {
	this->vertexShader = vertexShader;
}
	
void Program::setFragmentShader(Shader* fragmentShader) {
	this->fragmentShader = fragmentShader;
}

namespace {
	void compileShader(uint& id, u8* source, int length, ShaderType type) {
		char* shaderSource = new char[length + 1];
		for (int i = 0; i < length; ++i) shaderSource[i] = reinterpret_cast<char*>(source)[i];
		shaderSource[length] = 0;
		const char* cShaderSource = shaderSource;
		id = glCreateShader(type == VertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
		glShaderSource(id, 1, &cShaderSource, 0);
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

void Program::link(const VertexStructure& structure) {
	compileShader(vertexShader->id, vertexShader->source, vertexShader->length, VertexShader);
	compileShader(fragmentShader->id, fragmentShader->source, fragmentShader->length, FragmentShader);
	glAttachShader(programId, vertexShader->id);
	glAttachShader(programId, fragmentShader->id);
		
	int index = 0;
	for (int i = 0; i < structure.size; ++i) {
		VertexElement element = structure.elements[i];
		glBindAttribLocation(programId, index, element.name);
		++index;
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
}
	
void Program::set() {
	glUseProgram(programId);
	for (int index = 0; index < textureCount; ++index) glUniform1i(textureValues[index], index);
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.location = glGetUniformLocation(programId, name);
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
		index = textureCount;
		textureValues[index] = location;
		textures[index] = name;
		++textureCount;
	}
	TextureUnit unit;
	unit.unit = index;
	return unit;
}
