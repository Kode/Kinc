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

namespace Kore {
#if !defined(KORE_IOS) && !defined(KORE_ANDROID)
	extern bool programUsesTessellation;
#endif
	bool supportsConservativeRaster = false;

#ifdef KORE_WINDOWS
	namespace System {
		extern int currentDeviceId;
	}
#endif
}

namespace {
	void __stdcall debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		Kore::log(Info, "OpenGL: %s", message);
	}

#ifdef KORE_WINDOWS
	HINSTANCE instance = 0;
	HDC deviceContexts[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	HGLRC glContexts[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	Graphics4::RenderTarget* windowRenderTargets[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	Graphics4::VertexBuffer* windowVertexBuffer;
	Graphics4::IndexBuffer* windowIndexBuffer;
	Graphics4::PipelineState* windowPipeline;
	bool glewInitialized = false;

	void initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits) {
		HWND windowHandle = (HWND)System::windowHandle(window);

#ifndef VR_RIFT
		PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
		                             1,
		                             PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		                             PFD_TYPE_RGBA,
		                             32,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             0,
		                             (BYTE)depthBufferBits,
		                             (BYTE)stencilBufferBits,
		                             0,
		                             PFD_MAIN_PLANE,
		                             0,
		                             0,
		                             0,
		                             0};

		deviceContexts[window] = GetDC(windowHandle);
		GLuint pixelFormat = ChoosePixelFormat(deviceContexts[window], &pfd);
		SetPixelFormat(deviceContexts[window], pixelFormat, &pfd);
		HGLRC tempGlContext = wglCreateContext(deviceContexts[window]);
		wglMakeCurrent(deviceContexts[window], tempGlContext);
		Kore::System::currentDeviceId = window;

		if (!glewInitialized) {
			glewInit();
			glewInitialized = true;
		}

		if (wglewIsSupported("WGL_ARB_create_context") == 1) {
			int attributes[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
			                    4,
			                    WGL_CONTEXT_MINOR_VERSION_ARB,
			                    2,
			                    WGL_CONTEXT_FLAGS_ARB,
			                    WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			                    WGL_CONTEXT_PROFILE_MASK_ARB,
			                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			                    0};

			glContexts[window] = wglCreateContextAttribsARB(deviceContexts[window], glContexts[0], attributes);
			glCheckErrors();
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(tempGlContext);
			wglMakeCurrent(deviceContexts[window], glContexts[window]);
			glCheckErrors();
		}
		else {
			glContexts[window] = tempGlContext;
		}

		if (System::hasShowWindowFlag()) {
			ShowWindow(windowHandle, SW_SHOW);
			SetForegroundWindow(windowHandle); // slightly higher priority
			SetFocus(windowHandle);
		}
#else
		deviceContexts[window] = GetDC(windowHandle);
		glContexts[window] = wglGetCurrentContext();
		if (!glewInitialized) {
			glewInit();
			glewInitialized = true;
		}
#endif

		if (window != 0) {
			wglShareLists(glContexts[0], glContexts[window]);
			windowRenderTargets[window] = new Graphics4::RenderTarget(800, 600, depthBufferBits);
			if (windowVertexBuffer == nullptr) {
				wglMakeCurrent(deviceContexts[window], glContexts[window]);
				Graphics4::VertexStructure structure;
				structure.add("pos", Graphics4::Float2VertexData);
				windowVertexBuffer = new Graphics4::VertexBuffer(4, structure);
				float* vertices = windowVertexBuffer->lock();
				vertices[0] = -1.0f;
				vertices[1] = -1.0f;
				vertices[2] = -1.0f;
				vertices[3] = 1.0f;
				vertices[4] = 1.0f;
				vertices[5] = 1.0f;
				vertices[6] = 1.0f;
				vertices[7] = -1.0f;
				windowVertexBuffer->unlock();
				
				windowIndexBuffer = new Graphics4::IndexBuffer(6);
				int* indices = windowIndexBuffer->lock();
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 2;
				indices[3] = 0;
				indices[4] = 2;
				indices[5] = 3;
				windowIndexBuffer->unlock();

				Graphics4::Shader* windowVertexShader = new Graphics4::Shader(
					"#version 450\n"\
					"in vec2 pos;\n"\
					"out vec2 texCoord;\n"\
					"void main() {\n"\
						"gl_Position = vec4(pos, 0.5, 1.0);\n"\
						"texCoord = (pos + 1.0) / 2.0;\n"\
					"}\n",
					Graphics4::VertexShader);

				Graphics4::Shader* windowFragmentShader = new Graphics4::Shader(
					"#version 450\n"\
					"uniform sampler2D tex;\n"\
					"in vec2 texCoord;\n"\
					"out vec4 frag;\n"\
					"void main() {\n"\
						"frag = texture(tex, texCoord);\n"\
						"frag = vec4(1.0, 0.0, 0.0, 1.0);\n"\
					"}\n",
					Graphics4::FragmentShader);

				windowPipeline = new Graphics4::PipelineState();
				windowPipeline->inputLayout[0] = &structure;
				windowPipeline->inputLayout[1] = nullptr;
				windowPipeline->vertexShader = windowVertexShader;
				windowPipeline->fragmentShader = windowFragmentShader;
				windowPipeline->compile();

				wglMakeCurrent(deviceContexts[0], glContexts[0]);
			}
		}
	}
#endif

	Graphics4::TextureFilter minFilters[10][32];
	Graphics4::MipmapFilter mipFilters[10][32];
	int originalFramebuffer[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	uint arrayId[10];

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

void Graphics4::destroy(int window) {
#ifdef KORE_WINDOWS
	if (glContexts[window]) {
		affirm(wglMakeCurrent(nullptr, nullptr));
		affirm(wglDeleteContext(glContexts[window]));
		glContexts[window] = nullptr;
	}

	HWND windowHandle = (HWND)System::windowHandle(window);

	// TODO (DK) shouldn't 'deviceContexts[windowId] = nullptr;' be moved out of here?
	if (deviceContexts[window] && !ReleaseDC(windowHandle, deviceContexts[window])) {
		// MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		deviceContexts[window] = nullptr;
	}
#endif

	System::destroyWindow(window);
}

#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef KORE_WINDOWS
void Graphics4::setup() {}
#endif

static void initGLState(int window) {
#ifndef VR_RIFT
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, System::windowWidth(window), System::windowHeight(window));
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFramebuffer[window]);

	for (int i = 0; i < 32; ++i) {
		minFilters[window][i] = Graphics4::LinearFilter;
		mipFilters[window][i] = Graphics4::NoMipFilter;
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

#ifdef KORE_IOS
	glGenVertexArraysOES(1, &arrayId[windowId]);
	glCheckErrors();
#elif !defined(KORE_ANDROID) && !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_PI)
	glGenVertexArrays(1, &arrayId[windowId]);
	glCheckErrors();
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

void Graphics4::changeResolution(int width, int height) {
	_width = width;
	_height = height;
	if (renderToBackbuffer) {
		_renderTargetWidth = _width;
		_renderTargetHeight = _height;
	}
}

// TODO (DK) should return displays refreshrate?
unsigned Graphics4::refreshRate() {
	return 60;
}

bool Graphics4::vsynced() {
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

bool Graphics4::swapBuffers(int contextId) {
#ifdef KORE_WINDOWS

	for (int i = 9; i >= 0; --i) {
		if (deviceContexts[i] != nullptr) {
			wglMakeCurrent(deviceContexts[i], glContexts[i]);
			if (i != 0) {
				restoreRenderTarget();
				clear(Graphics4::ClearColorFlag, 0xff00ffff);
				setPipeline(windowPipeline);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, windowRenderTargets[i]->_texture);
				setIndexBuffer(*windowIndexBuffer);
				setVertexBuffer(*windowVertexBuffer);
				drawIndexedVertices();
			}
			::SwapBuffers(deviceContexts[i]);
		}
	}
#else
	System::swapBuffers(contextId);
#endif
	return true;
}

#ifdef KORE_IOS
void beginGL();
#endif

#ifdef KORE_WINDOWS
void Graphics4::makeCurrent(int contextId) {
	wglMakeCurrent(deviceContexts[contextId], glContexts[contextId]);
}
#endif

void Graphics4::begin(int contextId) {
	if (System::currentDevice() != -1) {
		if (System::currentDevice() != contextId) {
			//log(Warning, "begin: wrong glContext is active");
		}
		else {
			//**log(Warning, "begin: a glContext is still active");
		}

		// return; // TODO (DK) return here?
	}

	// System::setCurrentDevice(contextId);
	// System::makeCurrent(contextId);

	if (contextId == 0) {
		restoreRenderTarget();
	}
	else {
		setRenderTarget(windowRenderTargets[contextId]);
	}
	
	glViewport(0, 0, _width, _height);

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

	/*void glCheckErrors() {
	    if (System::currentDevice() == -1) {
	        log(Warning, "no OpenGL device context is set");
	        return;
	    }

	//#ifdef _DEBUG
	    GLenum code = glGetError();
	    while (code != GL_NO_ERROR) {
	        //std::printf("GLError: %s\n", glewGetErrorString(code));
	        switch (code) {
	        case GL_INVALID_VALUE:
	            log(Warning, "OpenGL: Invalid value");
	            break;
	        case GL_INVALID_OPERATION:
	            log(Warning, "OpenGL: Invalid operation");
	            break;
	        default:
	            log(Warning, "OpenGL: Error code %i", code);
	            break;
	        }
	        code = glGetError();
	    }
	//#endif
	}*/

#ifdef KORE_WINDOWS
void Graphics4::clearCurrent() {
	wglMakeCurrent(nullptr, nullptr);
}
#endif

// TODO (DK) this never gets called on some targets, needs investigation?
void Graphics4::end(int windowId) {
	// glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	// glClear(GL_COLOR_BUFFER_BIT);
	glCheckErrors();

	if (System::currentDevice() == -1) {
		//log(Warning, "end: a glContext wasn't active");
	}

	if (System::currentDevice() != windowId) {
		//log(Warning, "end: wrong glContext is active");
	}

	// System::clearCurrent();
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
#if defined(KORE_IOS)
	glBindVertexArrayOES(arrayId[0]);
	glCheckErrors();
#elif !defined(KORE_ANDROID) && !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_PI)
	glBindVertexArray(arrayId[System::currentDevice()]);
	glCheckErrors();
#endif

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
		switch (minFilters[System::currentDevice()][unit]) {
		case Graphics4::PointFilter:
			switch (mipFilters[System::currentDevice()][unit]) {
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
			switch (mipFilters[System::currentDevice()][unit]) {
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
			if (minFilters[System::currentDevice()][unit] == Graphics4::AnisotropicFilter) {
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
	minFilters[System::currentDevice()][texunit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.unit);
}

void Graphics4::setTexture3DMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	minFilters[System::currentDevice()][texunit.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.unit);
#endif
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[System::currentDevice()][texunit.unit] = filter;
	setMinMipFilters(GL_TEXTURE_2D, texunit.unit);
}

void Graphics4::setTexture3DMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[System::currentDevice()][texunit.unit] = filter;
#ifndef KORE_OPENGL_ES
	setMinMipFilters(GL_TEXTURE_3D, texunit.unit);
#endif
}

void Graphics4::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Graphics4::setRenderTargets(RenderTarget** targets, int count) {
	// TODO (DK) uneccessary?
	// System::makeCurrent(texture->contextId);
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
	glBindFramebuffer(GL_FRAMEBUFFER, originalFramebuffer[System::currentDevice()]);
	glCheckErrors();
	int w = System::windowWidth(System::currentDevice());
	int h = System::windowHeight(System::currentDevice());
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
