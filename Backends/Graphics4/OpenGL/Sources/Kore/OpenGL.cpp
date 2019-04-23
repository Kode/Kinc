#include "pch.h"

#include "OpenGL.h"
#include "VertexBufferImpl.h"
#include "ogl.h"

#include <Kinc/Graphics4/IndexBuffer.h>
#include <Kinc/Graphics4/PipelineState.h>
#include <Kinc/Graphics4/RenderTarget.h>
#include <Kinc/Graphics4/Texture.h>
#include <Kinc/Graphics4/TextureArray.h>
#include <Kinc/Graphics4/VertexBuffer.h>

#include <Kinc/Error.h>
#include <Kinc/Log.h>
#include <Kinc/Math/Core.h>
#include <Kinc/System.h>
#include <Kinc/Window.h>

#include "OpenGLWindow.h"

#include <Kore/Windows.h>

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

	Kinc_G4_TextureFilter minFilters[32];
	Kinc_G4_MipmapFilter mipFilters[32];

	int _width;
	int _height;
	int _renderTargetWidth;
	int _renderTargetHeight;
	bool renderToBackbuffer;

	Kinc_G4_PipelineState *lastPipeline = nullptr;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	void *glesDrawBuffers;
#endif

	int texModesU[256];
	int texModesV[256];
}

void Kinc_Internal_Resize(int window, int width, int height) {}

void Kinc_Internal_ChangeFramebuffer(int window, Kinc_FramebufferOptions *frame) {
#ifdef KORE_WINDOWS
	if (window == 0) {
#ifdef KORE_VR
		vsync = false;
#endif
		if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(frame->vertical_sync);
	}
#endif
}

void Kinc_G4_Destroy(int window) {
#ifdef KORE_WINDOWS
	if (Kinc_Internal_windows[window].glContext) {
		assert(wglMakeCurrent(nullptr, nullptr));
		assert(wglDeleteContext(Kinc_Internal_windows[window].glContext));
		Kinc_Internal_windows[window].glContext = nullptr;
	}

	HWND windowHandle = Kinc_Windows_WindowHandle(window);

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

void Kinc_G4_Init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
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

	_width = Kinc_WindowWidth(0);
	_height = Kinc_WindowHeight(0);
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

	lastPipeline = nullptr;

#if defined(KORE_LINUX) || defined(KORE_MACOS)
	unsigned vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glCheckErrors();
	glBindVertexArray(vertexArray);
	glCheckErrors();
#endif
}

bool Kinc_WindowVSynced(int window) {
#ifdef KORE_WINDOWS
	return wglGetSwapIntervalEXT();
#else
	return true;
#endif
}

void Kinc_G4_SetBool(Kinc_G4_ConstantLocation location, bool value) {
	glUniform1i(location.impl.location, value ? 1 : 0);
	glCheckErrors();
}

void Kinc_G4_SetInt(Kinc_G4_ConstantLocation location, int value) {
	glUniform1i(location.impl.location, value);
	glCheckErrors();
}

void Kinc_G4_SetFloat(Kinc_G4_ConstantLocation location, float value) {
	glUniform1f(location.impl.location, value);
	glCheckErrors();
}

void Kinc_G4_SetFloat2(Kinc_G4_ConstantLocation location, float value1, float value2) {
	glUniform2f(location.impl.location, value1, value2);
	glCheckErrors();
}

void Kinc_G4_SetFloat3(Kinc_G4_ConstantLocation location, float value1, float value2, float value3) {
	glUniform3f(location.impl.location, value1, value2, value3);
	glCheckErrors();
}

void Kinc_G4_SetFloat4(Kinc_G4_ConstantLocation location, float value1, float value2, float value3, float value4) {
	glUniform4f(location.impl.location, value1, value2, value3, value4);
	glCheckErrors();
}

void Kinc_G4_SetFloats(Kinc_G4_ConstantLocation location, float *values, int count) {
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

void Kinc_G4_SetMatrix4(Kinc_G4_ConstantLocation location, Kinc_Matrix4x4 *value) {
	glUniformMatrix4fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

void Kinc_G4_SetMatrix3(Kinc_G4_ConstantLocation location, Kinc_Matrix3x3 *value) {
	glUniformMatrix3fv(location.impl.location, 1, GL_FALSE, value->m);
	glCheckErrors();
}

extern "C" Kinc_G4_IndexBuffer *Kinc_Internal_CurrentIndexBuffer;

void Kinc_G4_DrawIndexedVertices() {
	Kinc_G4_DrawIndexedVerticesFromTo(0, Kinc_G4_IndexBuffer_Count(Kinc_Internal_CurrentIndexBuffer));
}

void Kinc_G4_DrawIndexedVerticesFromTo(int start, int count) {
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

void Kinc_G4_DrawIndexedVerticesInstanced(int instanceCount) {
	Kinc_G4_DrawIndexedVerticesInstancedFromTo(instanceCount, 0, Kinc_G4_IndexBuffer_Count(Kinc_Internal_CurrentIndexBuffer));
}

void Kinc_G4_DrawIndexedVerticesInstancedFromTo(int instanceCount, int start, int count) {
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

bool Kinc_G4_SwapBuffers() {
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

void Kinc_G4_Begin(int window) {
	currentWindow = window;
	Kinc_Internal_setWindowRenderTarget(window);

#ifdef KORE_IOS
	beginGL();
#endif

	glViewport(0, 0, Kinc_WindowWidth(window), Kinc_WindowHeight(window));

#ifdef KORE_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void Kinc_G4_Viewport(int x, int y, int width, int height) {
	glViewport(x, _renderTargetHeight - y - height, width, height);
}

void Kinc_G4_Scissor(int x, int y, int width, int height) {
	glEnable(GL_SCISSOR_TEST);
	if (renderToBackbuffer) {
		glScissor(x, _renderTargetHeight - y - height, width, height);
	}
	else {
		glScissor(x, y, width, height);
	}
}

void Kinc_G4_DisableScissor() {
	glDisable(GL_SCISSOR_TEST);
}

void Kinc_G4_End(int windowId) {
	currentWindow = 0;
	glCheckErrors();
}

void Kinc_G4_Clear(unsigned flags, unsigned color, float depth, int stencil) {
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
		Kinc_G4_SetPipeline(lastPipeline);
	}
}

void Kinc_G4_SetVertexBuffers(Kinc_G4_VertexBuffer **vertexBuffers, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += Kinc_Internal_G4_VertexBuffer_Set(vertexBuffers[i], offset);
	}
}

void Kinc_G4_SetIndexBuffer(Kinc_G4_IndexBuffer *indexBuffer) {
	Kinc_Internal_G4_IndexBuffer_Set(indexBuffer);
}

extern "C" void Kinc_G4_Internal_TextureSet(Kinc_G4_Texture *texture, Kinc_G4_TextureUnit unit);
extern "C" void Kinc_G4_Internal_TextureImageSet(Kinc_G4_Texture *texture, Kinc_G4_TextureUnit unit);

void Kinc_G4_SetTexture(Kinc_G4_TextureUnit unit, Kinc_G4_Texture *texture) {
	Kinc_G4_Internal_TextureSet(texture, unit);
}

void Kinc_G4_SetImageTexture(Kinc_G4_TextureUnit unit, Kinc_G4_Texture *texture) {
	Kinc_G4_Internal_TextureImageSet(texture, unit);
}

namespace {
	void setTextureAddressingInternal(GLenum target, Kinc_G4_TextureUnit unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing) {
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

int Kinc_G4_Internal_TextureAddressingU(Kinc_G4_TextureUnit unit) {
	return texModesU[unit.impl.unit];
}

int Kinc_G4_Internal_TextureAddressingV(Kinc_G4_TextureUnit unit) {
	return texModesV[unit.impl.unit];
}

void Kinc_G4_SetTextureAddressing(Kinc_G4_TextureUnit unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing) {
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
}

void Kinc_G4_SetTexture3DAddressing(Kinc_G4_TextureUnit unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing) {
#ifndef KORE_OPENGL_ES
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

namespace {
	void setTextureMagnificationFilterInternal(GLenum target, Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter) {
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

void Kinc_G4_SetTextureMagnificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter) {
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, texunit, filter);
}

void Kinc_G4_SetTexture3DMagnificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter) {
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

void Kinc_G4_SetTextureMinificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter) {
	minFilters[texunit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.impl.unit);
}

void Kinc_G4_SetTexture3DMinificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter) {
	minFilters[texunit.impl.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.impl.unit);
#endif
}

void Kinc_G4_SetTextureMipmapFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_MipmapFilter filter) {
	mipFilters[texunit.impl.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.impl.unit);
}

void Kinc_G4_SetTexture3DMipmapFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_MipmapFilter filter) {
	mipFilters[texunit.impl.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.impl.unit);
#endif
}

void Kinc_G4_SetTextureCompareMode(Kinc_G4_TextureUnit texunit, bool enabled) {
	if (texunit.impl.unit < 0) return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void Kinc_G4_etCubeMapCompareMode(Kinc_G4_TextureUnit texunit, bool enabled) {
	if (texunit.impl.unit < 0) return;
	if (enabled) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	}
	else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void Kinc_G4_SetTextureOperation(Kinc_G4_TextureOperation operation, Kinc_G4_TextureArgument arg1, Kinc_G4_TextureArgument arg2) {
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Kinc_G4_SetRenderTargets(Kinc_G4_RenderTarget **targets, int count) {
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

void Kinc_G4_SetRenderTargetFace(Kinc_G4_RenderTarget *texture, int face) {
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

void Kinc_G4_RestoreRenderTarget() {
	Kinc_Internal_setWindowRenderTarget(currentWindow);
	glCheckErrors();
	int w = Kinc_WindowWidth(currentWindow);
	int h = Kinc_WindowHeight(currentWindow);
	glViewport(0, 0, w, h);
	_renderTargetWidth = w;
	_renderTargetHeight = h;
	renderToBackbuffer = true;
	glCheckErrors();
}

bool Kinc_G4_RenderTargetsInvertedY() {
	return true;
}

bool Kinc_G4_NonPow2TexturesSupported() {
	return true;
}

#if (defined(KORE_OPENGL) && !defined(KORE_PI) && !defined(KORE_ANDROID)) || (defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
bool Kinc_G4_InitOcclusionQuery(unsigned *occlusionQuery) {
	glGenQueries(1, occlusionQuery);
	return true;
}

void Kinc_G4_DeleteOcclusionQuery(unsigned occlusionQuery) {
	glDeleteQueries(1, &occlusionQuery);
}

#if defined(KORE_OPENGL_ES)
#define SAMPLES_PASSED GL_ANY_SAMPLES_PASSED
#else
#define SAMPLES_PASSED GL_SAMPLES_PASSED
#endif

void Kinc_G4_RenderOcclusionQuery(unsigned occlusionQuery, int triangles) {
	glBeginQuery(SAMPLES_PASSED, occlusionQuery);
	glDrawArrays(GL_TRIANGLES, 0, triangles);
	glCheckErrors();
	glEndQuery(SAMPLES_PASSED);
}

bool Kinc_G4_AreQueryResultsAvailable(unsigned occlusionQuery) {
	unsigned available;
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT_AVAILABLE, &available);
	return available != 0;
}

void Kinc_G4_GetQueryResults(unsigned occlusionQuery, unsigned *pixelCount) {
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT, pixelCount);
}
#endif

void Kinc_G4_Flush() {
	glFlush();
	glCheckErrors();
}

void Kinc_G4_SetPipeline(Kinc_G4_PipelineState *pipeline) {
	Kinc_G4_Internal_SetPipeline(pipeline);
	lastPipeline = pipeline;
}

void Kinc_G4_SetStencilReferenceValue(int value) {
	glStencilFunc(Kinc_G4_Internal_StencilFunc(lastPipeline->stencilMode), value, lastPipeline->stencilReadMask);
}

extern "C" void Kinc_G4_Internal_TextureArraySet(Kinc_G4_TextureArray *array, Kinc_G4_TextureUnit unit); 

void Kinc_G4_SetTextureArray(Kinc_G4_TextureUnit unit, Kinc_G4_TextureArray *array) {
	Kinc_G4_Internal_TextureArraySet(array, unit);
}

int Kinc_G4_Internal_StencilFunc(Kinc_G4_CompareMode mode) {
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
