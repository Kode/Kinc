#include <kinc/compute/compute.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/image.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include <kinc/backend/graphics4/ogl.h>

#include <stdio.h>
#include <string.h>

#if defined(KORE_WINDOWS) || (defined(KORE_LINUX) && defined(GL_VERSION_4_3)) || (defined(KORE_ANDROID) && defined(GL_ES_VERSION_3_1))
#define HAS_COMPUTE
bool kinc_internal_gl_has_compute = true;
#else
bool kinc_internal_gl_has_compute = false;
#endif

#ifdef HAS_COMPUTE
static int convertInternalImageFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return GL_RGBA32F;
	case KINC_IMAGE_FORMAT_RGBA64:
		return GL_RGBA16F;
	case KINC_IMAGE_FORMAT_RGBA32:
	default:
		return GL_RGBA8;
	case KINC_IMAGE_FORMAT_A32:
		return GL_R32F;
	case KINC_IMAGE_FORMAT_A16:
		return GL_R16F;
	case KINC_IMAGE_FORMAT_GREY8:
		return GL_R8;
	}
}

static int convertInternalRTFormat(kinc_g4_render_target_format_t format) {
	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return GL_RGBA16F;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return GL_R32F;
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return GL_RGBA32F;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH:
		return GL_DEPTH_COMPONENT16;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
		return GL_RED;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return GL_R16F;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		return GL_RGBA;
	}
}

static void setTextureAddressingInternal(GLenum target, kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir,
                                         kinc_g4_texture_addressing_t addressing) {
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	GLenum texDir = dir == KINC_G4_TEXTURE_DIRECTION_U ? GL_TEXTURE_WRAP_S : (KINC_G4_TEXTURE_DIRECTION_V ? GL_TEXTURE_WRAP_T : GL_TEXTURE_WRAP_R);
	switch (addressing) {
	case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
		glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
		break;
	case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
	default:
		glTexParameteri(target, texDir, GL_REPEAT);
		break;
	}f
	glCheckErrors();
}

static void setTextureMagnificationFilterInternal(GLenum target, kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
	default:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
	glCheckErrors();
}

static kinc_g4_texture_filter_t minFilters[32] = {KINC_G4_TEXTURE_FILTER_POINT};
static kinc_g4_mipmap_filter_t mipFilters[32] = {KINC_G4_MIPMAP_FILTER_NONE};

static void setMinMipFilters(GLenum target, int unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
	switch (minFilters[unit]) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (mipFilters[unit]) {
		case KINC_G4_MIPMAP_FILTER_NONE:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_POINT:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_LINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		switch (mipFilters[unit]) {
		case KINC_G4_MIPMAP_FILTER_NONE:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case KINC_G4_MIPMAP_FILTER_POINT:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;
		case KINC_G4_MIPMAP_FILTER_LINEAR:
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		}
		if (minFilters[unit] == KINC_G4_TEXTURE_FILTER_ANISOTROPIC) {
			float maxAniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
			glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
		}
		break;
	}
	glCheckErrors();
}
#endif

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *source, int length) {
	shader->impl._length = length;
	shader->impl.textureCount = 0;
	shader->impl._source = (char *)malloc(sizeof(char) * (length + 1));
	for (int i = 0; i < length; ++i) {
		shader->impl._source[i] = ((char *)source)[i];
	}
	shader->impl._source[length] = 0;

#ifdef HAS_COMPUTE
	shader->impl._id = glCreateShader(GL_COMPUTE_SHADER);
	glCheckErrors();
	glShaderSource(shader->impl._id, 1, (const GLchar **)&shader->impl._source, NULL);
	glCompileShader(shader->impl._id);

	int result;
	glGetShaderiv(shader->impl._id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetShaderiv(shader->impl._id, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(sizeof(char) * length);
		glGetShaderInfoLog(shader->impl._id, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL compiler error: %s\n", errormessage);
		free(errormessage);
	}

	shader->impl._programid = glCreateProgram();
	glAttachShader(shader->impl._programid, shader->impl._id);
	glLinkProgram(shader->impl._programid);

	glGetProgramiv(shader->impl._programid, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(shader->impl._programid, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(sizeof(char) * length);
		glGetProgramInfoLog(shader->impl._programid, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL linker error: %s\n", errormessage);
		free(errormessage);
	}
#endif

	// TODO: Get rid of allocations
	shader->impl.textures = (char **)malloc(sizeof(char *) * 16);
	for (int i = 0; i < 16; ++i) {
		shader->impl.textures[i] = (char *)malloc(sizeof(char) * 128);
		shader->impl.textures[i][0] = 0;
	}
	shader->impl.textureValues = (int *)malloc(sizeof(int) * 16);
}

void kinc_compute_shader_destroy(kinc_compute_shader_t *shader) {
	free(shader->impl._source);
	shader->impl._source = NULL;
#ifdef HAS_COMPUTE
	glDeleteProgram(shader->impl._programid);
	glDeleteShader(shader->impl._id);
#endif
}
kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_constant_location_t location;
#ifdef HAS_COMPUTE
	location.impl.location = glGetUniformLocation(shader->impl._programid, name);
	location.impl.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(shader->impl._programid, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(shader->impl._programid, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.impl.type = type;
			break;
		}
	}
	glCheckErrors();
	if (location.impl.location < 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}
#endif
	return location;
}

static int findTexture(kinc_compute_shader_t *shader, const char *name) {
	for (int index = 0; index < shader->impl.textureCount; ++index) {
		if (strcmp(shader->impl.textures[index], name) == 0) return index;
	}
	return -1;
}

kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name) {
	int index = findTexture(shader, name);
	if (index < 0) {
		int location = glGetUniformLocation(shader->impl._programid, name);
		glCheckErrors();
		index = shader->impl.textureCount;
		shader->impl.textureValues[index] = location;
		strcpy(shader->impl.textures[index], name);
		++shader->impl.textureCount;
	}
	kinc_compute_texture_unit_t unit;
	unit.impl.unit = index;
	return unit;
}

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value) {
#ifdef HAS_COMPUTE
	glUniform1i(location.impl.location, value ? 1 : 0);
	glCheckErrors();
#endif
}

void kinc_compute_set_int(kinc_compute_constant_location_t location, int value) {
#ifdef HAS_COMPUTE
	glUniform1i(location.impl.location, value);
	glCheckErrors();
#endif
}

void kinc_compute_set_float(kinc_compute_constant_location_t location, float value) {
#ifdef HAS_COMPUTE
	glUniform1f(location.impl.location, value);
	glCheckErrors();
#endif
}

void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2) {
#ifdef HAS_COMPUTE
	glUniform2f(location.impl.location, value1, value2);
	glCheckErrors();
#endif
}

void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3) {
#ifdef HAS_COMPUTE
	glUniform3f(location.impl.location, value1, value2, value3);
	glCheckErrors();
#endif
}

void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4) {
#ifdef HAS_COMPUTE
	glUniform4f(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
#endif
}

void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count) {
#ifdef HAS_COMPUTE
	switch (location.impl.type) {
	case GL_FLOAT_VEC2:
		glUniform2fv(location.impl.location, count / 2, values);
		break;
	case GL_FLOAT_VEC3:
		glUniform3fv(location.impl.location, count / 3, values);
		break;
	case GL_FLOAT_VEC4:
		glUniform4fv(location.impl.location, count / 4, values);
		break;
	case GL_FLOAT_MAT4:
		glUniformMatrix4fv(location.impl.location, count / 16, false, values);
		break;
	default:
		glUniform1fv(location.impl.location, count, values);
		break;
	}
	glCheckErrors();
#endif
}

void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, kinc_matrix4x4_t *value) {
#ifdef HAS_COMPUTE
	glUniformMatrix4fv(location.impl.location, 1, GL_FALSE, &value->m[0]);
	glCheckErrors();
#endif
}

void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, kinc_matrix3x3_t *value) {
#ifdef HAS_COMPUTE
	glUniformMatrix3fv(location.impl.location, 1, GL_FALSE, &value->m[0]);
	glCheckErrors();
#endif
}

void kinc_compute_set_buffer(kinc_shader_storage_buffer_t *buffer, int index) {
#ifdef HAS_COMPUTE
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer->impl.bufferId);
	glCheckErrors();
#endif
}

void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture_t *texture, kinc_compute_access_t access) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	glCheckErrors();
	GLenum glaccess = access == KINC_COMPUTE_ACCESS_READ ? GL_READ_ONLY : (access == KINC_COMPUTE_ACCESS_WRITE ? GL_WRITE_ONLY : GL_READ_WRITE);
	glBindImageTexture(unit.impl.unit, texture->impl.texture, 0, GL_FALSE, 0, glaccess, convertInternalImageFormat(texture->format));
	glCheckErrors();
#endif
}

void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *texture, kinc_compute_access_t access) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	glCheckErrors();
	GLenum glaccess = access == KINC_COMPUTE_ACCESS_READ ? GL_READ_ONLY : (access == KINC_COMPUTE_ACCESS_WRITE ? GL_WRITE_ONLY : GL_READ_WRITE);
	glBindImageTexture(unit.impl.unit, texture->impl._texture, 0, GL_FALSE, 0, glaccess,
	                   convertInternalRTFormat((kinc_g4_render_target_format_t)texture->impl.format));
	glCheckErrors();
#endif
}

void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture_t *texture) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	glCheckErrors();
	GLenum gltarget = texture->tex_depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(gltarget, texture->impl.texture);
	glCheckErrors();
#endif
}

void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *target) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	glCheckErrors();
	glBindTexture(target->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, target->impl._texture);
	glCheckErrors();
#endif
}

void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *target) {
#ifdef HAS_COMPUTE
	glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
	glCheckErrors();
	glBindTexture(target->isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, target->impl._depthTexture);
	glCheckErrors();
#endif
}

void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
#ifdef HAS_COMPUTE
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
#endif
}

void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
#ifdef HAS_COMPUTE
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
#ifdef HAS_COMPUTE
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, unit, filter);
#endif
}

void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
#ifdef HAS_COMPUTE
	setTextureMagnificationFilterInternal(GL_TEXTURE_3D, unit, filter);
#endif
}

void kinc_compute_set_texture_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
#ifdef HAS_COMPUTE
	minFilters[unit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, unit.impl.unit);
#endif
}

void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
#ifdef HAS_COMPUTE
	minFilters[unit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_3D, unit.impl.unit);
#endif
}

void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {
#ifdef HAS_COMPUTE
	mipFilters[unit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, unit.impl.unit);
#endif
}

void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {
#ifdef HAS_COMPUTE
	mipFilters[unit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_3D, unit.impl.unit);
#endif
}

void kinc_compute_set_shader(kinc_compute_shader_t *shader) {
#ifdef HAS_COMPUTE
	glUseProgram(shader->impl._programid);
	glCheckErrors();

	for (int index = 0; index < shader->impl.textureCount; ++index) {
		glUniform1i(shader->impl.textureValues[index], index);
		glCheckErrors();
	}
#endif
}

void kinc_compute(int x, int y, int z) {
#ifdef HAS_COMPUTE
	glDispatchCompute(x, y, z);
	glCheckErrors();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glCheckErrors();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glCheckErrors();
#endif
}
