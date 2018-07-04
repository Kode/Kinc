#pragma once

#ifdef KORE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace Kore {
	namespace Graphics4 {
		class RenderTarget;
	}

	namespace OpenGL {
		struct Window {
			HDC deviceContext;
			HGLRC glContext;
			int framebuffer;
			uint vertexArray;
			Graphics4::RenderTarget* renderTarget;
		};

		extern Window windows[10];

		void initWindowsGLContext(int window, int depthBufferBits, int stencilBufferBits);
		void blitWindowContent(int window);
		void setWindowRenderTarget(int window);
	}
}
