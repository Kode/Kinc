#include "pch.h"

#include "OpenGL.h"
#include "VertexBufferImpl.h"
#include "ogl.h"

#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/TextureArray.h>

#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Window.h>

#include "OpenGLWindow.h"

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

using namespace Kore;
using namespace Kore::OpenGL;

namespace Kore {
#if !defined(KORE_IOS) && !defined(KORE_ANDROID)
	extern bool programUsesTessellation;
#endif
	bool supportsConservativeRaster = false;
}

namespace {
	void __stdcall debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		Kore::log(Info, "OpenGL: %s", message);
	}

#ifdef KORE_WINDOWS
	HINSTANCE instance = 0;
#endif

	int currentWindow = 0;

	Graphics4::TextureFilter minFilters[32];
	Graphics4::MipmapFilter mipFilters[32];
	
	int _width;
	int _height;
	int _renderTargetWidth;
	int _renderTargetHeight;
	bool renderToBackbuffer;

	Graphics4::PipelineState* lastPipeline = nullptr;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	void* glesDrawBuffers;
#endif
}

void Graphics4::_resize(int window, int width, int height) {

}

void Graphics4::_changeFramebuffer(int window, FramebufferOptions* frame) {
#ifdef KORE_WINDOWS
	if (window == 0) {
#ifdef KORE_VR
		vsync = false;
#endif
		if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(frame->verticalSync);
	}
#endif
}

void Graphics4::destroy(int window) {
#ifdef KORE_WINDOWS
	if (windows[window].glContext) {
		affirm(wglMakeCurrent(nullptr, nullptr));
		affirm(wglDeleteContext(windows[window].glContext));
		windows[window].glContext = nullptr;
	}

	HWND windowHandle = Window::get(window)->_data.handle;

	if (windows[window].deviceContext != nullptr) {
		ReleaseDC(windowHandle, windows[window].deviceContext);
		windows[window].deviceContext = nullptr;
	}
#endif
}

#ifdef CreateWindow
#undef CreateWindow
#endif

static void initGLState(int window) {
#ifndef VR_RIFT
	for (int i = 0; i < 32; ++i) {
		minFilters[i] = Graphics4::LinearFilter;
		mipFilters[i] = Graphics4::NoMipFilter;
	}
#endif
}

void Graphics4::init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
#ifdef KORE_WINDOWS
	initWindowsGLContext(windowId, depthBufferBits, stencilBufferBits);
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

	_width = System::windowWidth(0);
	_height = System::windowHeight(0);
	_renderTargetWidth = _width;
	_renderTargetHeight = _height;
	renderToBackbuffer = true;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	glesDrawBuffers = (void*)eglGetProcAddress("glDrawBuffers");
#endif

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debugCallback, nullptr);

#ifndef KORE_OPENGL_ES
	int extensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensions);
	for (int i = 0; i < extensions; ++i) {
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (extension != nullptr && strcmp(extension, "GL_NV_conservative_raster") == 0) {
			supportsConservativeRaster = true;
		}
	}
#endif

	lastPipeline = nullptr;
}

bool Kore::Window::vsynced() {
#ifdef KORE_WINDOWS
	return wglGetSwapIntervalEXT();
#else
	return true;
#endif
}

void Graphics4::setBool(ConstantLocation location, bool value) {
	glUniform1i(location.location, value ? 1 : 0);
	glCheckErrors();
}

void Graphics4::setInt(ConstantLocation location, int value) {
	glUniform1i(location.location, value);
	glCheckErrors();
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	glUniform1f(location.location, value);
	glCheckErrors();
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	glUniform2f(location.location, value1, value2);
	glCheckErrors();
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	glUniform3f(location.location, value1, value2, value3);
	glCheckErrors();
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	glUniform4f(location.location, value1, value2, value3, value4);
	glCheckErrors();
}

void Graphics4::setFloats(ConstantLocation location, float* values, int count) {
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
	case GL_FLOAT_MAT4:
		glUniformMatrix4fv(location.location, count / 16, false, values);
		break;
	default:
		glUniform1fv(location.location, count, values);
		break;
	}
	glCheckErrors();
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	glUniformMatrix4fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors();
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	glUniformMatrix3fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors();
}

void Graphics4::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics4::drawIndexedVertices(int start, int count) {
#ifdef KORE_OPENGL_ES
#if defined(KORE_ANDROID) || defined(KORE_PI)
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void*)(start * sizeof(GL_UNSIGNED_SHORT)));
#else
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
#endif
	glCheckErrors();
#else
	if (programUsesTessellation) {
		glDrawElements(GL_PATCHES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
	else {
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
#endif
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {
	drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
#ifndef KORE_OPENGL_ES
	if (programUsesTessellation) {
		glDrawElementsInstanced(GL_PATCHES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
	else {
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
#endif
}

bool Graphics4::swapBuffers() {
#ifdef KORE_WINDOWS
	for (int i = 9; i >= 0; --i) {
		if (windows[i].deviceContext != nullptr) {
			wglMakeCurrent(windows[i].deviceContext, windows[i].glContext);
			if (i != 0) {
				blitWindowContent(i);
			}
			::SwapBuffers(windows[i].deviceContext);
		}
	}
	wglMakeCurrent(windows[0].deviceContext, windows[0].glContext);
#else
	System::swapBuffers(contextId);
#endif
	return true;
}

#ifdef KORE_IOS
void beginGL();
#endif

void Graphics4::begin(int window) {
	currentWindow = window;
	setWindowRenderTarget(window);
	
	glViewport(0, 0, System::windowWidth(window), System::windowHeight(window));

#ifdef KORE_IOS
	beginGL();
#endif

#ifdef KORE_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void Graphics4::viewport(int x, int y, int width, int height) {
	glViewport(x, _renderTargetHeight - y - height, width, height);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	glEnable(GL_SCISSOR_TEST);
	if (renderToBackbuffer) {
		glScissor(x, _renderTargetHeight - y - height, width, height);
	}
	else {
		glScissor(x, y, width, height);
	}
}

void Graphics4::disableScissor() {
	glDisable(GL_SCISSOR_TEST);
}

void Graphics4::end(int windowId) {
	currentWindow = 0;
	glCheckErrors();
}

void Graphics4::clear(uint flags, uint color, float depth, int stencil) {
	glColorMask(true, true, true, true);
	glCheckErrors();
	glClearColor(((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	             ((color & 0xff000000) >> 24) / 255.0f);
	glCheckErrors();
	if (flags & ClearDepthFlag) {
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
	GLbitfield oglflags = ((flags & ClearColorFlag) ? GL_COLOR_BUFFER_BIT : 0) | ((flags & ClearDepthFlag) ? GL_DEPTH_BUFFER_BIT : 0) |
	                      ((flags & ClearStencilFlag) ? GL_STENCIL_BUFFER_BIT : 0);
	glClear(oglflags);
	glCheckErrors();
	if (lastPipeline != nullptr) {
		setPipeline(lastPipeline);
	}
}

void Graphics4::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += vertexBuffers[i]->_set(offset);
	}
}

void Graphics4::setIndexBuffer(IndexBuffer& indexBuffer) {
	indexBuffer._set();
}

void Graphics4::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

void Graphics4::setImageTexture(TextureUnit unit, Texture* texture) {
	texture->_setImage(unit);
}

namespace {
	void setTextureAddressingInternal(GLenum target, Graphics4::TextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
		glActiveTexture(GL_TEXTURE0 + unit.unit);
		GLenum texDir;
		switch (dir) {
		case Graphics4::U:
			texDir = GL_TEXTURE_WRAP_S;
			break;
		case Graphics4::V:
			texDir = GL_TEXTURE_WRAP_T;
			break;
		case Graphics4::W:
#ifndef KORE_OPENGL_ES
			texDir = GL_TEXTURE_WRAP_R;
#endif
			break;
		}
		switch (addressing) {
		case Graphics4::Clamp:
			glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
			break;
		case Graphics4::Repeat:
			glTexParameteri(target, texDir, GL_REPEAT);
			break;
		case Graphics4::Border:
			// unsupported
			glTexParameteri(target, texDir, GL_CLAMP_TO_EDGE);
			break;
		case Graphics4::Mirror:
			// unsupported
			glTexParameteri(target, texDir, GL_REPEAT);
			break;
		}
		glCheckErrors();
	}
} // namespace

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	setTextureAddressingInternal(GL_TEXTURE_2D, unit, dir, addressing);
}

void Graphics4::setTexture3DAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
#ifndef KORE_OPENGL_ES
	setTextureAddressingInternal(GL_TEXTURE_3D, unit, dir, addressing);
#endif
}

namespace {
	void setTextureMagnificationFilterInternal(GLenum target, Graphics4::TextureUnit texunit, Graphics4::TextureFilter filter) {
		glActiveTexture(GL_TEXTURE0 + texunit.unit);
		glCheckErrors();
		switch (filter) {
		case Graphics4::PointFilter:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		case Graphics4::LinearFilter:
		case Graphics4::AnisotropicFilter:
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		}
		glCheckErrors();
	}
} // namespace

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	setTextureMagnificationFilterInternal(GL_TEXTURE_2D, texunit, filter);
}

void Graphics4::setTexture3DMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
#ifndef KORE_OPENGL_ES
	setTextureMagnificationFilterInternal(GL_TEXTURE_3D, texunit, filter);
#endif
}

namespace {
	void setMinMipFilters(GLenum target, int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glCheckErrors();
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
} // namespace

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	minFilters[texunit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.unit);
}

void Graphics4::setTexture3DMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	minFilters[texunit.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.unit);
#endif
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[texunit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.unit);
}

void Graphics4::setTexture3DMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[texunit.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.unit);
#endif
}

void Graphics4::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Graphics4::setRenderTargets(RenderTarget** targets, int count) {
	glBindFramebuffer(GL_FRAMEBUFFER, targets[0]->_framebuffer);
	glCheckErrors();
#ifndef KORE_OPENGL_ES
	if (targets[0]->isCubeMap)
		glFramebufferTexture(GL_FRAMEBUFFER, targets[0]->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, targets[0]->_texture, 0); // Layered
#endif
	glViewport(0, 0, targets[0]->width, targets[0]->height);
	_renderTargetWidth = targets[0]->width;
	_renderTargetHeight = targets[0]->height;
	renderToBackbuffer = false;
	glCheckErrors();

	if (count > 1) {
		for (int i = 0; i < count; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, targets[i]->_texture, 0);
			glCheckErrors();
		}

		GLenum buffers[16];
		for (int i = 0; i < count; ++i) buffers[i] = GL_COLOR_ATTACHMENT0 + i;
#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
		((void (*)(GLsizei, GLenum*))glesDrawBuffers)(count, buffers);
#elif !defined(KORE_OPENGL_ES)
		glDrawBuffers(count, buffers);
#endif
		glCheckErrors();
	}
}

void Graphics4::setRenderTargetFace(RenderTarget* texture, int face) {
	glBindFramebuffer(GL_FRAMEBUFFER, texture->_framebuffer);
	glCheckErrors();
	glFramebufferTexture2D(GL_FRAMEBUFFER, texture->isDepthAttachment ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
	                       texture->_texture, 0);
	glViewport(0, 0, texture->width, texture->height);
	_renderTargetWidth = texture->width;
	_renderTargetHeight = texture->height;
	renderToBackbuffer = false;
	glCheckErrors();
}

void Graphics4::restoreRenderTarget() {
	setWindowRenderTarget(currentWindow);
	glCheckErrors();
	int w = System::windowWidth(currentWindow);
	int h = System::windowHeight(currentWindow);
	glViewport(0, 0, w, h);
	_renderTargetWidth = w;
	_renderTargetHeight = h;
	renderToBackbuffer = true;
	glCheckErrors();
}

bool Graphics4::renderTargetsInvertedY() {
	return true;
}

bool Graphics4::nonPow2TexturesSupported() {
	return true;
}

#if (defined(KORE_OPENGL) && !defined(KORE_PI) && !defined(KORE_ANDROID)) || (defined(KORE_ANDROID) && KORE_ANDROID_API >= 18)
bool Graphics4::initOcclusionQuery(uint* occlusionQuery) {
	glGenQueries(1, occlusionQuery);
	return true;
}

void Graphics4::deleteOcclusionQuery(uint occlusionQuery) {
	glDeleteQueries(1, &occlusionQuery);
}

#if defined(KORE_OPENGL_ES)
#define SAMPLES_PASSED GL_ANY_SAMPLES_PASSED
#else
#define SAMPLES_PASSED GL_SAMPLES_PASSED
#endif

void Graphics4::renderOcclusionQuery(uint occlusionQuery, int triangles) {
	glBeginQuery(SAMPLES_PASSED, occlusionQuery);
	glDrawArrays(GL_TRIANGLES, 0, triangles);
	glCheckErrors();
	glEndQuery(SAMPLES_PASSED);
}

bool Graphics4::isQueryResultsAvailable(uint occlusionQuery) {
	uint available;
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT_AVAILABLE, &available);
	return available != 0;
}

void Graphics4::getQueryResults(uint occlusionQuery, uint* pixelCount) {
	glGetQueryObjectuiv(occlusionQuery, GL_QUERY_RESULT, pixelCount);
}
#endif

void Graphics4::flush() {
	glFlush();
	glCheckErrors();
}

void Graphics4::setPipeline(PipelineState* pipeline) {
	pipeline->set(pipeline);
	lastPipeline = pipeline;
}

void Graphics4::setTextureArray(TextureUnit unit, TextureArray* array) {
	array->set(unit);
}
