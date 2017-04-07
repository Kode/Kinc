#include "pch.h"

#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Core.h>

#include <string.h>

using namespace Kore;

ShaderImpl::ShaderImpl(void* data, int length) : length(length), id(0) {
	char* source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		source[i] = ((char*)data)[i];
	}
	source[length] = 0;
	this->source = source;
}

ShaderImpl::ShaderImpl(const char* source) : source(source), length(strlen(source)), id(0) {

}

ShaderImpl::~ShaderImpl() {
	delete[] source;
	source = nullptr;
	if (id != 0) glDeleteShader(id);
}

Graphics4::Shader::Shader(void* data, int length, ShaderType type) : ShaderImpl(data, length) {}

Graphics4::Shader::Shader(const char* source, ShaderType type) : ShaderImpl(source) {}
