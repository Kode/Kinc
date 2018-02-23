#include "pch.h"

#include "ComputeImpl.h"
#include "ogl.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Core.h>
#include <stdio.h>
#include <string.h>

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

	int convertInternalFormat(Graphics4::RenderTargetFormat format) {
		switch (format) {
		case Graphics4::Target64BitFloat:
			return GL_RGBA16F;
		case Graphics4::Target32BitRedFloat:
			return GL_R32F;
		case Graphics4::Target128BitFloat:
			return GL_RGBA32F;
		case Graphics4::Target16BitDepth:
			return GL_DEPTH_COMPONENT16;
		case Graphics4::Target8BitRed:
			return GL_RED;
		case Graphics4::Target16BitRedFloat:
			return GL_R16F;
		case Graphics4::Target32Bit:
		default:
			return GL_RGBA;
		}
	}

	void setTextureAddressingInternal(GLenum target, ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
		glActiveTexture(GL_TEXTURE0 + unit.unit);
		GLenum texDir = dir == Graphics4::U ? GL_TEXTURE_WRAP_S : (Graphics4::V ? GL_TEXTURE_WRAP_T : GL_TEXTURE_WRAP_R);
		switch (addressing) {
		case Graphics4::Clamp:
			glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
			break;
		case Graphics4::Repeat:
		default:
			glTexParameteri(target, texDir, GL_REPEAT);
			break;
		}
		glCheckErrors();
	}

	void setTextureMagnificationFilterInternal(GLenum target, ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
		glActiveTexture(GL_TEXTURE0 + unit.unit);
		switch (filter) {
		case Graphics4::PointFilter:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case Graphics4::LinearFilter:
		case Graphics4::AnisotropicFilter:
		default:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
		glCheckErrors();
	}

	Graphics4::TextureFilter minFilters[32] = { Graphics4::PointFilter };
	Graphics4::MipmapFilter mipFilters[32] = { Graphics4::NoMipFilter };

	void setMinMipFilters(GLenum target, int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		switch (minFilters[unit]) {
		case Graphics4::PointFilter:
			switch (mipFilters[unit]) {
			case Graphics4::NoMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				break;
			case Graphics4::PointMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case Graphics4::LinearMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				break;
			}
			break;
		case Graphics4::LinearFilter:
		case Graphics4::AnisotropicFilter:
			switch (mipFilters[unit]) {
			case Graphics4::NoMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			case Graphics4::PointMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case Graphics4::LinearMipFilter:
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
			}
			if (minFilters[unit] == Graphics4::AnisotropicFilter) {
				float maxAniso = 0.0f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
				glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
			}
			break;
		}
		glCheckErrors();
	}
#endif
}

ComputeShaderImpl::ComputeShaderImpl(void* source, int length) : _length(length), textureCount(0) {
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

	// TODO: Get rid of allocations
	textures = new char*[16];
	for (int i = 0; i < 16; ++i) {
		textures[i] = new char[128];
		textures[i][0] = 0;
	}
	textureValues = new int[16];
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
	location.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(_programid, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(_programid, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.type = type;
			break;
		}
	}
	glCheckErrors2();
	if (location.location < 0) {
		log(Warning, "Uniform %s not found.", name);
	}
#endif
	return location;
}

int ComputeShaderImpl::findTexture(const char* name) {
	for (int index = 0; index < textureCount; ++index) {
		if (strcmp(textures[index], name) == 0) return index;
	}
	return -1;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	int index = findTexture(name);
	if (index < 0) {
		int location = glGetUniformLocation(_programid, name);
		glCheckErrors2();
		index = textureCount;
		textureValues[index] = location;
		strcpy(textures[index], name);
		++textureCount;
	}
	ComputeTextureUnit unit;
	unit.unit = index;
	return unit;
}

void Compute::setBool(ComputeConstantLocation location, bool value) {
#ifdef HAS_COMPUTE
	glUniform1i(location.location, value ? 1 : 0);
	glCheckErrors2();
#endif
}

void Compute::setInt(ComputeConstantLocation location, int value) {
#ifdef HAS_COMPUTE
	glUniform1i(location.location, value);
	glCheckErrors2();
#endif
}

void Compute::setFloat(ComputeConstantLocation location, float value) {
#ifdef HAS_COMPUTE
	glUniform1f(location.location, value);
	glCheckErrors2();
#endif
}

void Compute::setFloat2(ComputeConstantLocation location, float value1, float value2) {
#ifdef HAS_COMPUTE
	glUniform2f(location.location, value1, value2);
	glCheckErrors2();
#endif
}

void Compute::setFloat3(ComputeConstantLocation location, float value1, float value2, float value3) {
#ifdef HAS_COMPUTE
	glUniform3f(location.location, value1, value2, value3);
	glCheckErrors2();
#endif
}

void Compute::setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4) {
#ifdef HAS_COMPUTE
	glUniform4f(location.location, value1, value2, value3, value4);
	glCheckErrors2();
#endif
}

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {
#ifdef HAS_COMPUTE
	switch (location.type) {
	case GL_FLOAT_VEC2:
		glUniform2fv(location.location, count / 2, values);
		break;
	case GL_FLOAT_VEC3:
		glUniform3fv(location.location, count / 3, values);
		break;
	case GL_FLOAT_VEC4:
		glUniform4fv(location.location, count / 4, values);
		break;
	default:
		glUniform1fv(location.location, count, values);
		break;
	}
	glCheckErrors2();
#endif
}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {
#ifdef HAS_COMPUTE
	glUniformMatrix4fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors2();
#endif
}

void Compute::setMatrix(ComputeConstantLocation location, const mat3& value) {
#ifdef HAS_COMPUTE
	glUniformMatrix3fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors2();
#endif
}

void Compute::setBuffer(ShaderStorageBuffer* buffer, int index) {
#ifdef HAS_COMPUTE
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer->bufferId); glCheckErrors2();
#endif
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	GLenum glaccess = access == Read ? GL_READ_ONLY : (access == Write ? GL_WRITE_ONLY : GL_READ_WRITE);
	glBindImageTexture(unit.unit, texture->texture, 0, GL_FALSE, 0, glaccess, convertInternalFormat(texture->format));
	glCheckErrors2();
#endif
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target, Access access) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	GLenum glaccess = access == Read ? GL_READ_ONLY : (access == Write ? GL_WRITE_ONLY : GL_READ_WRITE);
	glBindImageTexture(unit.unit, target->_texture, 0, GL_FALSE, 0, glaccess, convertInternalFormat((Graphics4::RenderTargetFormat)target->format));
	glCheckErrors2();
#endif
}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	GLenum gltarget = texture->depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(gltarget, texture->texture);
	glCheckErrors2();
#endif
}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	glBindTexture(target->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, target->_texture);
	glCheckErrors2();
#endif
}

void Compute::setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors2();
	glBindTexture(target->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, target->_depthTexture);
	glCheckErrors2();
#endif
}

void Compute::setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
#ifdef HAS_COMPUTE
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
#endif
}

void Compute::setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
#ifdef HAS_COMPUTE
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

void Compute::setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
#ifdef HAS_COMPUTE
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, unit, filter);
#endif
}

void Compute::setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
#ifdef HAS_COMPUTE
	setTextureMagnificationFilterInternal(GL_TEXTURE_3D, unit, filter);
#endif
}

void Compute::setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
#ifdef HAS_COMPUTE
	minFilters[unit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, unit.unit);
#endif
}

void Compute::setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
#ifdef HAS_COMPUTE
	minFilters[unit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_3D, unit.unit);
#endif
}

void Compute::setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {
#ifdef HAS_COMPUTE
	mipFilters[unit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, unit.unit);
#endif
}

void Compute::setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {
#ifdef HAS_COMPUTE
	mipFilters[unit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_3D, unit.unit);
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
