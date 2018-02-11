#include "pch.h"

#include "ComputeImpl.h"
#include "ogl.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Core.h>
#include <stdio.h>

using namespace Kore;

#if defined(KORE_WINDOWS) || (defined(KORE_LINUX) && defined(GL_VERSION_4_3)) || (defined(KORE_ANDROID) && defined(GL_ES_VERSION_3_1))
#define HAS_COMPUTE
#endif

namespace {
#ifdef HAS_COMPUTE
	int convertInternalFormat(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA128:
			return GL_RGBA32F;
		case Graphics4::Image::RGBA64:
			return GL_RGBA16F;
		case Graphics4::Image::RGBA32:
		default:
			return GL_RGBA8;
		case Graphics4::Image::A32:
			return GL_R32F;
		case Graphics4::Image::A16:
			return GL_R16F;
		case Graphics4::Image::Grey8:
			return GL_R8;
		}
	}
#endif
}

ComputeShaderImpl::ComputeShaderImpl(void* source, int length) : _length(length) {
	_source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		_source[i] = ((char*)source)[i];
	}
	_source[length] = 0;

#ifdef HAS_COMPUTE
	_id = glCreateShader(GL_COMPUTE_SHADER);
	glCheckErrors();
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
#endif
}

ComputeShaderImpl::~ComputeShaderImpl() {
	delete[] _source;
	_source = nullptr;
#ifdef HAS_COMPUTE
	glDeleteProgram(_programid);
	glDeleteShader(_id);
#endif
}

ComputeShader::ComputeShader(void* _data, int length) : ComputeShaderImpl(_data, length) {}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
#ifdef HAS_COMPUTE
	location.location = glGetUniformLocation(_programid, name);
	glCheckErrors2();
	if (location.location < 0) {
		log(Warning, "Uniform %s not found.", name);
	}
#endif
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	unit.unit = 0;
	return unit;
}

void Compute::setFloat(ComputeConstantLocation location, float value) {
#ifdef HAS_COMPUTE
	glUniform1f(location.location, value);
	glCheckErrors2();
#endif
}

void Compute::setBuffer(ShaderStorageBuffer* buffer, int index) {
#ifdef HAS_COMPUTE
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer->bufferId); glCheckErrors2();
#endif
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Graphics4::Access access) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	GLenum glaccess = access == Graphics4::Access::Read ? GL_READ_ONLY : (access == Graphics4::Access::Write ? GL_WRITE_ONLY : GL_READ_WRITE);
	glBindImageTexture(0, texture->texture, 0, GL_FALSE, 0, glaccess, convertInternalFormat(texture->format));
	glCheckErrors2();
#endif
}

void Compute::setShader(ComputeShader* shader) {
#ifdef HAS_COMPUTE
	glUseProgram(shader->_programid);
	glCheckErrors2();
#endif
}

void Compute::compute(int x, int y, int z) {
#ifdef HAS_COMPUTE
	glDispatchCompute(x, y, z);
	glCheckErrors2();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glCheckErrors2();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glCheckErrors2();
#endif
}
