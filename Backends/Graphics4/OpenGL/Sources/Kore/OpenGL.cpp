#include "pch.h"

#include "OpenGL.h"
#include "VertexBufferImpl.h"
#include "ogl.h"

#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/texturearray.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include "OpenGLWindow.h"

#ifdef KORE_WINDOWS
#include <Kore/Windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef KORE_IOS
#include <OpenGLES/ES2/glext.h>
#endif

#ifdef KORE_WINDOWS
#include <GL/wglew.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#endif

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE 0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#endif
#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#endif

#if !defined(KORE_IOS) && !defined(KORE_ANDROID)
extern "C" bool Kinc_Internal_ProgramUsesTessellation;
#endif
extern "C" bool Kinc_Internal_SupportsConservativeRaster = false;
extern "C" bool Kinc_Internal_SupportsDepthTexture = true;

namespace {
#if defined(KORE_WINDOWS) && !defined(NDEBUG)
	void __stdcall debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
		kinc_log(KINC_LOG_LEVEL_INFO, "OpenGL: %s", message);
	}
#endif

#ifdef KORE_WINDOWS
	HINSTANCE instance = 0;
#endif

	int currentWindow = 0;

	kinc_g4_texture_filter_t minFilters[32];
	kinc_g4_mipmap_filter_t mipFilters[32];

	int _width;
	int _height;
	int _renderTargetWidth;
	int _renderTargetHeight;
	bool renderToBackbuffer;

	kinc_g4_pipeline_t *lastPipeline = nullptr;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	void *glesDrawBuffers;
#endif

	int texModesU[256];
	int texModesV[256];
}

extern "C" void kinc_internal_resize(int window, int width, int height) {}

extern "C" void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {
#ifdef KORE_WINDOWS
	if (window == 0) {
#ifdef KORE_VR
		vsync = false;
#endif
		if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(frame->vertical_sync);
	}
#endif
}

void kinc_g4_destroy(int window) {
#ifdef KORE_WINDOWS
	if (Kinc_Internal_windows[window].glContext) {
		assert(wglMakeCurrent(nullptr, nullptr));
		assert(wglDeleteContext(Kinc_Internal_windows[window].glContext));
		Kinc_Internal_windows[window].glContext = nullptr;
	}

	HWND windowHandle = kinc_windows_window_handle(window);

	if (Kinc_Internal_windows[window].deviceContext != nullptr) {
		ReleaseDC(windowHandle, Kinc_Internal_windows[window].deviceContext);
		Kinc_Internal_windows[window].deviceContext = nullptr;
	}
#endif
}

#ifdef CreateWindow
#undef CreateWindow
#endif

static void initGLState(int window) {
#ifndef VR_RIFT
	for (int i = 0; i < 32; ++i) {
		minFilters[i] = KINC_G4_TEXTURE_FILTER_LINEAR;
		mipFilters[i] = KINC_G4_MIPMAP_FILTER_NONE;
	}
#endif
}

void kinc_g4_init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
	for (int i = 0; i < 256; ++i) {
		texModesU[i] = GL_CLAMP_TO_EDGE;
		texModesV[i] = GL_CLAMP_TO_EDGE;
	}
#ifdef KORE_WINDOWS
	Kinc_Internal_initWindowsGLContext(windowId, depthBufferBits, stencilBufferBits);
#endif

	initGLState(windowId);

#ifdef KORE_WINDOWS
	if (windowId == 0) {
#ifdef KORE_VR
		vsync = false;
#endif
		if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(vsync);
	}
#endif

	_width = kinc_window_width(0);
	_height = kinc_window_height(0);
	_renderTargetWidth = _width;
	_renderTargetHeight = _height;
	renderToBackbuffer = true;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	glesDrawBuffers = (void *)eglGetProcAddress("glDrawBuffers");
#endif

#if defined(KORE_WINDOWS) && !defined(NDEBUG)
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debugCallback, nullptr);
#endif

#ifndef KORE_OPENGL_ES
	int extensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensions);
	for (int i = 0; i < extensions; ++i) {
		const char *extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
		if (extension != nullptr && strcmp(extension, "GL_NV_conservative_raster") == 0) {
			Kinc_Internal_SupportsConservativeRaster = true;
		}
	}
#endif

#ifdef KORE_OPENGL_ES
    char *exts = (char *)glGetString(GL_EXTENSIONS);
    Kinc_Internal_SupportsDepthTexture = exts != NULL && strstr(exts, "GL_OES_depth_texture") != NULL;
#endif

	lastPipeline = nullptr;

#if defined(KORE_LINUX) || defined(KORE_MACOS)
	unsigned vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glCheckErrors();
	glBindVertexArray(vertexArray);
	glCheckErrors();
#endif
}

bool kinc_window_vsynced(int window) {
#ifdef KORE_WINDOWS
	return wglGetSwapIntervalEXT();
#else
	return true;
#endif
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	glUniform1i(location.impl.location, value ? 1 : 0);
	glCheckErrors();
}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	glUniform1i(location.impl.location, value);
	glCheckErrors();
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {
	glUniform2i(location.impl.location, value1, value2);
	glCheckErrors();
}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {
	glUniform3i(location.impl.location, value1, value2, value3);
	glCheckErrors();
}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {
	glUniform4i(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	switch (location.impl.type) {
	case GL_INT_VEC2:
		glUniform2iv(location.impl.location, count / 2, values);
		break;
	case GL_INT_VEC3:
		glUniform3iv(location.impl.location, count / 3, values);
		break;
	case GL_INT_VEC4:
		glUniform4iv(location.impl.location, count / 4, values);
		break;
	default:
		glUniform1iv(location.impl.location, count, values);
		break;
	}
	glCheckErrors();
}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	glUniform1f(location.impl.location, value);
	glCheckErrors();
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	glUniform2f(location.impl.location, value1, value2);
	glCheckErrors();
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	glUniform3f(location.impl.location, value1, value2, value3);
	glCheckErrors();
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	glUniform4f(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
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
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	glUniformMatrix4fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	glUniformMatrix3fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

extern "C" kinc_g4_index_buffer_t *Kinc_Internal_CurrentIndexBuffer;

void kinc_g4_draw_indexed_vertices() {
	kinc_g4_draw_indexed_vertices_from_to(0, kinc_g4_index_buffer_count(Kinc_Internal_CurrentIndexBuffer));
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
#ifdef KORE_OPENGL_ES
#if defined(KORE_ANDROID) || defined(KORE_PI)
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void *)(start * sizeof(GL_UNSIGNED_SHORT)));
#else
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void *)(start * sizeof(GL_UNSIGNED_INT)));
#endif
	glCheckErrors();
#else
	if (Kinc_Internal_ProgramUsesTessellation) {
		glDrawElements(GL_PATCHES, count, GL_UNSIGNED_INT, (void *)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
	else {
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void *)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
#endif
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, 0, kinc_g4_index_buffer_count(Kinc_Internal_CurrentIndexBuffer));
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
#ifndef KORE_OPENGL_ES
	if (Kinc_Internal_ProgramUsesTessellation) {
		glDrawElementsInstanced(GL_PATCHES, count, GL_UNSIGNED_INT, (void *)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
	else {
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void *)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
#endif
}

#ifdef KORE_ANDROID
void androidSwapBuffers();
#endif

#ifdef KORE_LINUX
void swapLinuxBuffers(int window);
#endif

#ifdef KORE_MACOS
void swapBuffersMac(int window);
#endif

#ifdef KORE_IOS
void swapBuffersiOS();
#endif

bool kinc_g4_swap_buffers() {
#ifdef KORE_WINDOWS
	for (int i = 9; i >= 0; --i) {
		if (Kinc_Internal_windows[i].deviceContext != nullptr) {
			wglMakeCurrent(Kinc_Internal_windows[i].deviceContext, Kinc_Internal_windows[i].glContext);
			if (i != 0) {
				Kinc_Internal_blitWindowContent(i);
			}
			::SwapBuffers(Kinc_Internal_windows[i].deviceContext);
		}
	}
#elif defined(KORE_ANDROID)
	androidSwapBuffers();
#elif defined(KORE_LINUX)
	swapLinuxBuffers(0);
#elif defined(KORE_MACOS)
	swapBuffersMac(0);
#elif defined(KORE_IOS)
	swapBuffersiOS();
#endif
	return true;
}

#ifdef KORE_IOS
void beginGL();
#endif

void kinc_g4_begin(int window) {
	currentWindow = window;
	Kinc_Internal_setWindowRenderTarget(window);

#ifdef KORE_IOS
	beginGL();
#endif

	glViewport(0, 0, kinc_window_width(window), kinc_window_height(window));

#ifdef KORE_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	glViewport(x, _renderTargetHeight - y - height, width, height);
}

void kinc_g4_scissor(int x, int y, int width, int height) {
	glEnable(GL_SCISSOR_TEST);
	if (renderToBackbuffer) {
		glScissor(x, _renderTargetHeight - y - height, width, height);
	}
	else {
		glScissor(x, y, width, height);
	}
}

void kinc_g4_disable_scissor() {
	glDisable(GL_SCISSOR_TEST);
}

void kinc_g4_end(int windowId) {
	currentWindow = 0;
	glCheckErrors();
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	glColorMask(true, true, true, true);
	glCheckErrors();
	glClearColor(((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	             ((color & 0xff000000) >> 24) / 255.0f);
	glCheckErrors();
	if (flags & KINC_G4_CLEAR_DEPTH) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glCheckErrors();
	}
#ifdef KORE_OPENGL_ES
	glClearDepthf(depth);
#else
	glClearDepth(depth);
#endif
	glCheckErrors();
	glStencilMask(0xff);
	glCheckErrors();
	glClearStencil(stencil);
	glCheckErrors();
	GLbitfield oglflags = ((flags & KINC_G4_CLEAR_COLOR) ? GL_COLOR_BUFFER_BIT : 0) | ((flags & KINC_G4_CLEAR_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0) |
	                      ((flags & KINC_G4_CLEAR_STENCIL) ? GL_STENCIL_BUFFER_BIT : 0);
	glClear(oglflags);
	glCheckErrors();
	if (lastPipeline != nullptr) {
		kinc_g4_set_pipeline(lastPipeline);
	}
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **vertexBuffers, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += kinc_internal_g4_vertex_buffer_set(vertexBuffers[i], offset);
	}
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *indexBuffer) {
	kinc_internal_g4_index_buffer_set(indexBuffer);
}

extern "C" void Kinc_G4_Internal_TextureSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);
extern "C" void Kinc_G4_Internal_TextureImageSet(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	Kinc_G4_Internal_TextureSet(texture, unit);
}

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	Kinc_G4_Internal_TextureImageSet(texture, unit);
}

namespace {
	void setTextureAddressingInternal(GLenum target, kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
		glActiveTexture(GL_TEXTURE0 + unit.impl.unit);
		GLenum texDir;
		switch (dir) {
		case KINC_G4_TEXTURE_DIRECTION_U:
			texDir = GL_TEXTURE_WRAP_S;
			break;
		case KINC_G4_TEXTURE_DIRECTION_V:
			texDir = GL_TEXTURE_WRAP_T;
			break;
		case KINC_G4_TEXTURE_DIRECTION_W:
#ifndef KORE_OPENGL_ES
			texDir = GL_TEXTURE_WRAP_R;
#endif
			break;
		}
		switch (addressing) {
		case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
			glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
			if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
				texModesU[unit.impl.unit] = GL_CLAMP_TO_EDGE;
			}
			else {
				texModesV[unit.impl.unit] = GL_CLAMP_TO_EDGE;
			}
			break;
		case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
			glTexParameteri(target, texDir, GL_REPEAT);
			if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
				texModesU[unit.impl.unit] = GL_REPEAT;
			}
			else {
				texModesV[unit.impl.unit] = GL_REPEAT;
			}
			break;
		case KINC_G4_TEXTURE_ADDRESSING_BORDER:
			// unsupported
			glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
			if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
				texModesU[unit.impl.unit] = GL_CLAMP_TO_EDGE;
			}
			else {
				texModesV[unit.impl.unit] = GL_CLAMP_TO_EDGE;
			}
			break;
		case KINC_G4_TEXTURE_ADDRESSING_MIRROR:
			// unsupported
			glTexParameteri(target, texDir, GL_REPEAT);
			if (dir == KINC_G4_TEXTURE_DIRECTION_U) {
				texModesU[unit.impl.unit] = GL_REPEAT;
			}
			else {
				texModesV[unit.impl.unit] = GL_REPEAT;
			}
			break;
		}
		glCheckErrors();
	}
}

int Kinc_G4_Internal_TextureAddressingU(kinc_g4_texture_unit_t unit) {
	return texModesU[unit.impl.unit];
}

int Kinc_G4_Internal_TextureAddressingV(kinc_g4_texture_unit_t unit) {
	return texModesV[unit.impl.unit];
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
#ifndef KORE_OPENGL_ES
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

namespace {
	void setTextureMagnificationFilterInternal(GLenum target, kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
		glActiveTexture(GL_TEXTURE0 + texunit.impl.unit);
		glCheckErrors();
		switch (filter) {
		case KINC_G4_TEXTURE_FILTER_POINT:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case KINC_G4_TEXTURE_FILTER_LINEAR:
		case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
		glCheckErrors();
	}
} // namespace

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, texunit, filter);
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
#ifndef KORE_OPENGL_ES
	setTextureMagnificationFilterInternal(GL_TEXTURE_3D, texunit, filter);
#endif
}

namespace {
	void setMinMipFilters(GLenum target, int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glCheckErrors();
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
} // namespace

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	minFilters[texunit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.impl.unit);
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	minFilters[texunit.impl.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.impl.unit);
#endif
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	mipFilters[texunit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.impl.unit);
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	mipFilters[texunit.impl.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.impl.unit);
#endif
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t texunit, bool enabled) {
	if (texunit.impl.unit < 0) return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t texunit, bool enabled) {
	if (texunit.impl.unit < 0) return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void kinc_g4_set_texture_operation(kinc_g4_texture_operation_t operation, kinc_g4_texture_argument_t arg1, kinc_g4_texture_argument_t arg2) {
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void kinc_g4_set_render_targets(kinc_g4_render_target_t **targets, int count) {
	glBindFramebuffer(GL_FRAMEBUFFER, targets[0]->impl._framebuffer);
	glCheckErrors();
#ifndef KORE_OPENGL_ES
	if (targets[0]->isCubeMap)
		glFramebufferTexture(GL_FRAMEBUFFER, targets[0]->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, targets[0]->impl._texture,
		                     0); // Layered
#endif
	glViewport(0, 0, targets[0]->width, targets[0]->height);
	_renderTargetWidth = targets[0]->width;
	_renderTargetHeight = targets[0]->height;
	renderToBackbuffer = false;
	glCheckErrors();

	if (count > 1) {
		for (int i = 0; i < count; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, targets[i]->impl._texture, 0);
			glCheckErrors();
		}

		GLenum buffers[16];
		for (int i = 0; i < count; ++i) buffers[i] = GL_COLOR_ATTACHMENT0 + i;
#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
		((void (*)(GLsizei, GLenum *))glesDrawBuffers)(count, buffers);
#elif !defined(KORE_OPENGL_ES)
		glDrawBuffers(count, buffers);
#endif
		glCheckErrors();
	}
}

void kinc_g4_set_render_target_face(kinc_g4_render_target_t *texture, int face) {
	glBindFramebuffer(GL_FRAMEBUFFER, texture->impl._framebuffer);
	glCheckErrors();
	glFramebufferTexture2D(GL_FRAMEBUFFER, texture->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
	                       texture->impl._texture, 0);
	glViewport(0, 0, texture->width, texture->height);
	_renderTargetWidth = texture->width;
	_renderTargetHeight = texture->height;
	renderToBackbuffer = false;
	glCheckErrors();
}

void kinc_g4_restore_render_target() {
	Kinc_Internal_setWindowRenderTarget(currentWindow);
	glCheckErrors();
	int w = kinc_window_width(currentWindow);
	int h = kinc_window_height(currentWindow);
	glViewport(0, 0, w, h);
	_renderTargetWidth = w;
	_renderTargetHeight = h;
	renderToBackbuffer = true;
	glCheckErrors();
}

bool kinc_g4_render_targets_inverted_y() {
	return true;
}

bool kinc_g4_non_pow2_textures_supported() {
	return true;
}

#if (defined(KORE_OPENGL) && !defined(KORE_PI) && !defined(KORE_ANDROID)) || (defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery) {
	glGenQueries(1, occlusionQuery);
	return true;
}

void kinc_g4_delete_occlusion_query(unsigned occlusionQuery) {
	glDeleteQueries(1, &occlusionQuery);
}

#if defined(KORE_OPENGL_ES)
#define SAMPLES_PASSED GL_ANY_SAMPLES_PASSED
#else
#define SAMPLES_PASSED GL_SAMPLES_PASSED
#endif

void kinc_g4_render_occlusion_query(unsigned occlusionQuery, int triangles) {
	glBeginQuery(SAMPLES_PASSED, occlusionQuery);
	glDrawArrays(GL_TRIANGLES, 0, triangles);
	glCheckErrors();
	glEndQuery(SAMPLES_PASSED);
}

bool kinc_g4_are_query_results_available(unsigned occlusionQuery) {
	unsigned available;
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT_AVAILABLE, &available);
	return available != 0;
}

void kinc_g4_get_query_results(unsigned occlusionQuery, unsigned *pixelCount) {
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT, pixelCount);
}
#endif

void kinc_g4_flush() {
	glFlush();
	glCheckErrors();
}

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g4_internal_set_pipeline(pipeline);
	lastPipeline = pipeline;
}

void kinc_g4_set_stencil_reference_value(int value) {
	glStencilFunc(Kinc_G4_Internal_StencilFunc(lastPipeline->stencil_mode), value, lastPipeline->stencil_read_mask);
}

extern "C" void Kinc_G4_Internal_TextureArraySet(kinc_g4_texture_array *array, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, kinc_g4_texture_array_t *array) {
	Kinc_G4_Internal_TextureArraySet(array, unit);
}

int Kinc_G4_Internal_StencilFunc(kinc_g4_compare_mode_t mode) {
	switch (mode) {
	case KINC_G4_COMPARE_ALWAYS:
		return GL_ALWAYS;
	case KINC_G4_COMPARE_EQUAL:
		return GL_EQUAL;
	case KINC_G4_COMPARE_GREATER:
		return GL_GREATER;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		return GL_GEQUAL;
	case KINC_G4_COMPARE_LESS:
		return GL_LESS;
	case KINC_G4_COMPARE_LESS_EQUAL:
		return GL_LEQUAL;
	case KINC_G4_COMPARE_NEVER:
		return GL_NEVER;
	case KINC_G4_COMPARE_NOT_EQUAL:
		return GL_NOTEQUAL;
	}

	return 0;
}
