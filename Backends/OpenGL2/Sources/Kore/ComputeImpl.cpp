#include "pch.h"
#include "ComputeImpl.h"
#include <Kore/Compute/Compute.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Core.h>
#include "ogl.h"
#include <stdio.h>

using namespace Kore;

ComputeShaderImpl::ComputeShaderImpl(void* source, int length) : _length(length) {
	_source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		_source[i] = ((char*)source)[i];
	}
	_source[length] = 0;

	_id = glCreateShader(GL_COMPUTE_SHADER); glCheckErrors();
	glShaderSource(_id, 1, &_source, nullptr);
	glCompileShader(_id);

	int result;
	glGetShaderiv(_id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = new char[length];
		glGetShaderInfoLog(_id, length, nullptr, errormessage);
		log(Error, "GLSL compiler error: %s\n", errormessage);
		delete[] errormessage;
	}

	_programid = glCreateProgram();
	glAttachShader(_programid, _id);
	glLinkProgram(_programid);

	glGetProgramiv(_programid, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(_programid, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = new char[length];
		glGetProgramInfoLog(_programid, length, nullptr, errormessage);
		log(Error, "GLSL linker error: %s\n", errormessage);
		delete[] errormessage;
	}
}

ComputeShaderImpl::~ComputeShaderImpl() {
	delete[] _source;
	_source = nullptr;
	glDeleteProgram(_programid);
	glDeleteShader(_id);
}

ComputeShader::ComputeShader(void* _data, int length) : ComputeShaderImpl(_data, length) {

}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	location.location = glGetUniformLocation(_programid, name); glCheckErrors2();
	if (location.location < 0) {
		log(Warning, "Uniform %s not found.", name);
	}
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	unit.unit = 0;
	return unit;
}

void Compute::setFloat(ComputeConstantLocation location, float value) {
#if !defined(SYS_OSX) && !defined(SYS_IOS)
	glUniform1f(location.location, value); glCheckErrors2();
#endif
}

void Compute::setTexture(ComputeTextureUnit unit, Texture* texture) {
#if !defined(SYS_OSX) && !defined(SYS_IOS)
	glActiveTexture(GL_TEXTURE0 + unit.unit); glCheckErrors2();
	glBindImageTexture(0, texture->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); glCheckErrors2();
#endif
}

void Compute::setShader(ComputeShader* shader) {
#if !defined(SYS_OSX) && !defined(SYS_IOS)
	glUseProgram(shader->_programid); glCheckErrors2();
#endif
}

void Compute::compute(int x, int y, int z) {
#if !defined(SYS_OSX) && !defined(SYS_IOS)
	glDispatchCompute(x, y, z); glCheckErrors2();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); glCheckErrors2();
#endif
}
