#include "pch.h"

#include "OpenGLWindow.h"

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/Graphics4/IndexBuffer.h>
#include <Kinc/Graphics4/PipelineState.h>
#include <Kinc/Graphics4/RenderTarget.h>
#include <Kinc/Graphics4/Shader.h>
#include <Kinc/Graphics4/VertexBuffer.h>
#include <Kinc/System.h>
#include <Kinc/Window.h>

#include <Kore/Windows.h>

#include "ogl.h"

#ifdef KORE_WINDOWS
#include <GL/wglew.h>
#endif

Kinc_Internal_OpenGLWindow Kinc_Internal_windows[10] = {0};

static bool initialized = false;
static Kinc_G4_VertexBuffer windowVertexBuffer;
static Kinc_G4_IndexBuffer windowIndexBuffer;
static Kinc_G4_PipelineState windowPipeline;
static Kinc_G4_Shader windowVertexShader;
static Kinc_G4_Shader windowFragmentShader;
static bool glewInitialized = false;

#ifdef KORE_WINDOWS
void Kinc_Internal_initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits) {
	HWND windowHandle = Kinc_Windows_WindowHandle(window);

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

	Kinc_Internal_windows[window].deviceContext = GetDC(windowHandle);
	GLuint pixelFormat = ChoosePixelFormat(Kinc_Internal_windows[window].deviceContext, &pfd);
	SetPixelFormat(Kinc_Internal_windows[window].deviceContext, pixelFormat, &pfd);
	HGLRC tempGlContext = wglCreateContext(Kinc_Internal_windows[window].deviceContext);
	wglMakeCurrent(Kinc_Internal_windows[window].deviceContext, tempGlContext);

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

		Kinc_Internal_windows[window].glContext =
		    wglCreateContextAttribsARB(Kinc_Internal_windows[window].deviceContext, Kinc_Internal_windows[0].glContext, attributes);
		glCheckErrors();
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempGlContext);
		wglMakeCurrent(Kinc_Internal_windows[window].deviceContext, Kinc_Internal_windows[window].glContext);
		glCheckErrors();
	}
	else {
		Kinc_Internal_windows[window].glContext = tempGlContext;
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
		wglShareLists(Kinc_Internal_windows[0].glContext, Kinc_Internal_windows[window].glContext);
		wglMakeCurrent(Kinc_Internal_windows[0].deviceContext, Kinc_Internal_windows[0].glContext);
		Kinc_G4_RenderTarget_Create(Kinc_Internal_windows[window].renderTarget, Kinc_Windows_ManualWidth(window), Kinc_Windows_ManualHeight(window), depthBufferBits,
		                            false, KINC_G4_RENDER_TARGET_FORMAT_32BIT, -1, 0);
		if (!initialized) {
			wglMakeCurrent(Kinc_Internal_windows[window].deviceContext, Kinc_Internal_windows[window].glContext);
			Kinc_G4_VertexStructure structure;
			Kinc_G4_VertexStructure_Create(&structure);
			Kinc_G4_VertexStructure_Add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT2);
			Kinc_G4_VertexBuffer_Create(&windowVertexBuffer, 4, &structure, KINC_G4_USAGE_STATIC, 0);
			float* vertices = Kinc_G4_VertexBuffer_LockAll(&windowVertexBuffer);
			vertices[0] = -1.0f;
			vertices[1] = -1.0f;
			vertices[2] = -1.0f;
			vertices[3] = 1.0f;
			vertices[4] = 1.0f;
			vertices[5] = 1.0f;
			vertices[6] = 1.0f;
			vertices[7] = -1.0f;
			Kinc_G4_VertexBuffer_UnlockAll(&windowVertexBuffer);

			Kinc_G4_IndexBuffer_Create(&windowIndexBuffer, 6);
			int* indices = Kinc_G4_IndexBuffer_Lock(&windowIndexBuffer);
			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 2;
			indices[3] = 0;
			indices[4] = 2;
			indices[5] = 3;
			Kinc_G4_IndexBuffer_Unlock(&windowIndexBuffer);

			Kinc_G4_Shader_CreateFromSource(&windowVertexShader, "#version 450\n"
			                                                     "in vec2 pos;\n"
			                                                     "out vec2 texCoord;\n"
			                                                     "void main() {\n"
			                                                     "gl_Position = vec4(pos, 0.5, 1.0);\n"
			                                                     "texCoord = (pos + 1.0) / 2.0;\n"
			                                                     "}\n",
			                                                     KINC_SHADER_TYPE_VERTEX);

			Kinc_G4_Shader_CreateFromSource(&windowFragmentShader, "#version 450\n"
			                                                       "uniform sampler2D tex;\n"
			                                                       "in vec2 texCoord;\n"
			                                                       "out vec4 frag;\n"
			                                                       "void main() {\n"
			                                                       "frag = texture(tex, texCoord);\n"
			                                                       "}\n",
			                                                       KINC_SHADER_TYPE_FRAGMENT);

			Kinc_G4_PipelineState_Create(&windowPipeline);
			windowPipeline.inputLayout[0] = &structure;
			windowPipeline.inputLayout[1] = NULL;
			windowPipeline.vertexShader = &windowVertexShader;
			windowPipeline.fragmentShader = &windowFragmentShader;
			Kinc_G4_PipelineState_Compile(&windowPipeline);

			wglMakeCurrent(Kinc_Internal_windows[0].deviceContext, Kinc_Internal_windows[0].glContext);

			initialized = true;
		}
	}
	wglMakeCurrent(Kinc_Internal_windows[window].deviceContext, Kinc_Internal_windows[window].glContext);
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &Kinc_Internal_windows[window].framebuffer);
#ifdef KORE_IOS
	glGenVertexArraysOES(1, &arrayId[windowId]);
	glCheckErrors();
#elif !defined(KORE_ANDROID) && !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_PI)
	glGenVertexArrays(1, &Kinc_Internal_windows[window].vertexArray);
	glCheckErrors();
#endif
	wglMakeCurrent(Kinc_Internal_windows[0].deviceContext, Kinc_Internal_windows[0].glContext);
	glBindVertexArray(Kinc_Internal_windows[0].vertexArray);
	glCheckErrors();
}
#endif

void Kinc_Internal_blitWindowContent(int window) {
	glBindFramebuffer(GL_FRAMEBUFFER, Kinc_Internal_windows[window].framebuffer);
	Kinc_G4_Clear(KINC_G4_CLEAR_COLOR, 0xff00ffff, 0.0f, 0);
	Kinc_G4_SetPipeline(&windowPipeline);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Kinc_Internal_windows[window].renderTarget->impl._texture);
	Kinc_G4_SetIndexBuffer(&windowIndexBuffer);

#ifndef KORE_OPENGL_ES
	glBindVertexArray(Kinc_Internal_windows[window].vertexArray);
#endif
	glCheckErrors();
	Kinc_G4_VertexBuffer *vertexBuffers[1] = {&windowVertexBuffer};
	Kinc_G4_SetVertexBuffers(vertexBuffers, 1);

	Kinc_G4_DrawIndexedVertices();
	glCheckErrors();

#ifndef KORE_OPENGL_ES
	glBindVertexArray(Kinc_Internal_windows[0].vertexArray);
#endif
	glCheckErrors();
}

void Kinc_Internal_setWindowRenderTarget(int window) {
	if (window == 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, Kinc_Internal_windows[window].framebuffer);
	}
	else {
		Kinc_G4_RenderTarget *renderTargets[1] = {Kinc_Internal_windows[window].renderTarget};
		Kinc_G4_SetRenderTargets(renderTargets, 1);
	}
}
