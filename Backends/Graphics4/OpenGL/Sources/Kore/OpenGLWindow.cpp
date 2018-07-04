#include "pch.h"

#include "OpenGLWindow.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/System.h>

#include "ogl.h"

#ifdef KORE_WINDOWS
#include <GL/wglew.h>
#endif

using namespace Kore;

OpenGL::Window OpenGL::windows[10] = {0};

namespace {
	Graphics4::VertexBuffer* windowVertexBuffer;
	Graphics4::IndexBuffer* windowIndexBuffer;
	Graphics4::PipelineState* windowPipeline;
	bool glewInitialized = false;
}

void OpenGL::initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits) {
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

	windows[window].deviceContext = GetDC(windowHandle);
	GLuint pixelFormat = ChoosePixelFormat(windows[window].deviceContext, &pfd);
	SetPixelFormat(windows[window].deviceContext, pixelFormat, &pfd);
	HGLRC tempGlContext = wglCreateContext(windows[window].deviceContext);
	wglMakeCurrent(windows[window].deviceContext, tempGlContext);

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

		windows[window].glContext = wglCreateContextAttribsARB(windows[window].deviceContext, windows[0].glContext, attributes);
		glCheckErrors();
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tempGlContext);
		wglMakeCurrent(windows[window].deviceContext, windows[window].glContext);
		glCheckErrors();
	}
	else {
		windows[window].glContext = tempGlContext;
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
		wglShareLists(windows[0].glContext, windows[window].glContext);
		windows[window].renderTarget = new Graphics4::RenderTarget(800, 600, depthBufferBits);
		if (windowVertexBuffer == nullptr) {
			wglMakeCurrent(windows[window].deviceContext, windows[window].glContext);
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

			Graphics4::Shader* windowVertexShader = new Graphics4::Shader("#version 450\n"
			                                                              "in vec2 pos;\n"
			                                                              "out vec2 texCoord;\n"
			                                                              "void main() {\n"
			                                                              "gl_Position = vec4(pos, 0.5, 1.0);\n"
			                                                              "texCoord = (pos + 1.0) / 2.0;\n"
			                                                              "}\n",
			                                                              Graphics4::VertexShader);

			Graphics4::Shader* windowFragmentShader = new Graphics4::Shader("#version 450\n"
			                                                                "uniform sampler2D tex;\n"
			                                                                "in vec2 texCoord;\n"
			                                                                "out vec4 frag;\n"
			                                                                "void main() {\n"
			                                                                "frag = texture(tex, texCoord);\n"
			                                                                "}\n",
			                                                                Graphics4::FragmentShader);

			windowPipeline = new Graphics4::PipelineState();
			windowPipeline->inputLayout[0] = &structure;
			windowPipeline->inputLayout[1] = nullptr;
			windowPipeline->vertexShader = windowVertexShader;
			windowPipeline->fragmentShader = windowFragmentShader;
			windowPipeline->compile();

			wglMakeCurrent(windows[0].deviceContext, windows[0].glContext);
		}

		wglMakeCurrent(windows[window].deviceContext, windows[window].glContext);
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &windows[window].framebuffer);
#ifdef KORE_IOS
		glGenVertexArraysOES(1, &arrayId[windowId]);
		glCheckErrors();
#elif !defined(KORE_ANDROID) && !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_PI)
		glGenVertexArrays(1, &windows[window].vertexArray);
		glCheckErrors();
#endif
		wglMakeCurrent(windows[0].deviceContext, windows[0].glContext);
		glBindVertexArray(windows[0].vertexArray);
		glCheckErrors();
	}
}

void OpenGL::blitWindowContent(int window) {
	glBindFramebuffer(GL_FRAMEBUFFER, windows[window].framebuffer);
	Graphics4::clear(Graphics4::ClearColorFlag, 0xff00ffff);
	Graphics4::setPipeline(windowPipeline);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, windows[window].renderTarget->_texture);
	setIndexBuffer(*windowIndexBuffer);
	
	glBindVertexArray(windows[window].vertexArray);
	glCheckErrors();
	windowVertexBuffer->_set(0);

	Graphics4::drawIndexedVertices();
	glCheckErrors();

	glBindVertexArray(windows[0].vertexArray);
	glCheckErrors();
}

void OpenGL::setWindowRenderTarget(int window) {
	if (window == 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, windows[window].framebuffer);
	}
	else {
		Graphics4::setRenderTarget(windows[window].renderTarget);
	}
}
