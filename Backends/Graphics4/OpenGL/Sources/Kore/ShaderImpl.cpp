#include "pch.h"

#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Core.h>

#include <string.h>

using namespace Kore;

ShaderImpl::ShaderImpl(void* data, int length) : length(length), _glid(0) {
	char* source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		source[i] = ((char*)data)[i];
	}
	source[length] = 0;
	this->source = source;
}

ShaderImpl::ShaderImpl(const char* source) : source(source), length(strlen(source)), _glid(0) {}

ShaderImpl::~ShaderImpl() {
	delete[] source;
	source = nullptr;
	if (_glid != 0) glDeleteShader(_glid);
}

Graphics4::Shader::Shader(void* data, int length, ShaderType type) : ShaderImpl(data, length) {
	setId();
}

Graphics4::Shader::Shader(const char* source, ShaderType type) : ShaderImpl(source) {
	setId();
}
