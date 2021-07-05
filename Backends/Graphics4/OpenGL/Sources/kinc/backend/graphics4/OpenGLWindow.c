#include "OpenGLWindow.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/system.h>
#include <kinc/window.h>

#ifdef KORE_WINDOWS
#include <kinc/backend/Windows.h>
#endif

#include "ogl.h"

#ifdef KORE_WINDOWS
#include <GL/wglew.h>
#endif

Kinc_Internal_OpenGLWindow Kinc_Internal_windows[10] = {0};

static kinc_g4_vertex_buffer_t windowVertexBuffer;
static kinc_g4_index_buffer_t windowIndexBuffer;
static kinc_g4_pipeline_t windowPipeline;
#ifdef KORE_WINDOWS
static bool initialized = false;
static kinc_g4_shader_t windowVertexShader;
static kinc_g4_shader_t windowFragmentShader;
static bool glewInitialized = false;
#endif

#ifdef KORE_WINDOWS
void Kinc_Internal_initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits) {
	HWND windowHandle = kinc_windows_window_handle(window);

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
		kinc_g4_render_target_init(Kinc_Internal_windows[window].renderTarget, kinc_windows_manual_width(window), kinc_windows_manual_height(window),
		                           depthBufferBits, false, KINC_G4_RENDER_TARGET_FORMAT_32BIT, -1, 0);
		if (!initialized) {
			wglMakeCurrent(Kinc_Internal_windows[window].deviceContext, Kinc_Internal_windows[window].glContext);
			kinc_g4_vertex_structure_t structure;
			kinc_g4_vertex_structure_init(&structure);
			kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT2);
			kinc_g4_vertex_buffer_init(&windowVertexBuffer, 4, &structure, KINC_G4_USAGE_STATIC, 0);
			float *vertices = kinc_g4_vertex_buffer_lock_all(&windowVertexBuffer);
			vertices[0] = -1.0f;
			vertices[1] = -1.0f;
			vertices[2] = -1.0f;
			vertices[3] = 1.0f;
			vertices[4] = 1.0f;
			vertices[5] = 1.0f;
			vertices[6] = 1.0f;
			vertices[7] = -1.0f;
			kinc_g4_vertex_buffer_unlock_all(&windowVertexBuffer);

			kinc_g4_index_buffer_init(&windowIndexBuffer, 6, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
			int *indices = kinc_g4_index_buffer_lock(&windowIndexBuffer);
			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 2;
			indices[3] = 0;
			indices[4] = 2;
			indices[5] = 3;
			kinc_g4_index_buffer_unlock(&windowIndexBuffer);

			kinc_g4_shader_init_from_source(&windowVertexShader,
			                                "#version 450\n"
			                                "in vec2 pos;\n"
			                                "out vec2 texCoord;\n"
			                                "void main() {\n"
			                                "gl_Position = vec4(pos, 0.5, 1.0);\n"
			                                "texCoord = (pos + 1.0) / 2.0;\n"
			                                "}\n",
			                                KINC_G4_SHADER_TYPE_VERTEX);

			kinc_g4_shader_init_from_source(&windowFragmentShader,
			                                "#version 450\n"
			                                "uniform sampler2D tex;\n"
			                                "in vec2 texCoord;\n"
			                                "out vec4 frag;\n"
			                                "void main() {\n"
			                                "frag = texture(tex, texCoord);\n"
			                                "}\n",
			                                KINC_G4_SHADER_TYPE_FRAGMENT);

			kinc_g4_pipeline_init(&windowPipeline);
			windowPipeline.input_layout[0] = &structure;
			windowPipeline.input_layout[1] = NULL;
			windowPipeline.vertex_shader = &windowVertexShader;
			windowPipeline.fragment_shader = &windowFragmentShader;
			kinc_g4_pipeline_compile(&windowPipeline);

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
	kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0xff00ffff, 0.0f, 0);
	kinc_g4_set_pipeline(&windowPipeline);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Kinc_Internal_windows[window].renderTarget->impl._texture);
	kinc_g4_set_index_buffer(&windowIndexBuffer);

#ifndef KORE_OPENGL_ES
	glBindVertexArray(Kinc_Internal_windows[window].vertexArray);
#endif
	glCheckErrors();
	kinc_g4_vertex_buffer_t *vertexBuffers[1] = {&windowVertexBuffer};
	kinc_g4_set_vertex_buffers(vertexBuffers, 1);

	kinc_g4_draw_indexed_vertices();
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
		kinc_g4_render_target_t *renderTargets[1] = {Kinc_Internal_windows[window].renderTarget};
		kinc_g4_set_render_targets(renderTargets, 1);
	}
}
