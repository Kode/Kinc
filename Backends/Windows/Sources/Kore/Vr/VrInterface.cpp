#include "pch.h"

#include "VrInterface.h"

#ifdef VR_RIFT

#include "Kore/Log.h"

#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"

#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Kore {

	//-------------------------------------------------------------------------------------------
	struct OGL
	{
		HWND                    Window;
		HDC                     hDC;
		HGLRC                   WglContext;
		OVR::GLEContext         GLEContext;
		bool                    Running;
		bool                    Key[256];
		int                     WinSizeW;
		int                     WinSizeH;
		GLuint                  fboId;
		HINSTANCE               hInstance;

		static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
		{
			OGL *p = reinterpret_cast<OGL *>(GetWindowLongPtr(hWnd, 0));
			switch (Msg)
			{
			case WM_KEYDOWN:
				p->Key[wParam] = true;
				break;
			case WM_KEYUP:
				p->Key[wParam] = false;
				break;
			case WM_DESTROY:
				p->Running = false;
				break;
			default:
				return DefWindowProcW(hWnd, Msg, wParam, lParam);
			}
			if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE])
			{
				p->Running = false;
			}
			return 0;
		}

		OGL() :
			Window(nullptr),
			hDC(nullptr),
			WglContext(nullptr),
			GLEContext(),
			Running(false),
			WinSizeW(0),
			WinSizeH(0),
			fboId(0),
			hInstance(nullptr)
		{
			// Clear input
			for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
				Key[i] = false;
		}

		~OGL()
		{
			ReleaseDevice();
			CloseWindow();
		}

		bool InitWindow(HINSTANCE hInst, LPCWSTR title)
		{
			hInstance = hInst;
			Running = true;

			WNDCLASSW wc;
			memset(&wc, 0, sizeof(wc));
			wc.style = CS_CLASSDC;
			wc.lpfnWndProc = WindowProc;
			wc.cbWndExtra = sizeof(struct OGL *);
			wc.hInstance = GetModuleHandleW(NULL);
			wc.lpszClassName = L"ORT";
			RegisterClassW(&wc);

			// adjust the window size and show at InitDevice time
			//Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
			Window = CreateWindowA("ORT", "ORT(OpenGL)", WS_POPUP, 0, 0, 1000, 1000, GetDesktopWindow(), NULL, hInst, NULL);
			if (!Window) return false;

			SetWindowLongPtr(Window, 0, LONG_PTR(this));

			hDC = GetDC(Window);

			return true;
		}

		void CloseWindow()
		{
			if (Window)
			{
				if (hDC)
				{
					ReleaseDC(Window, hDC);
					hDC = nullptr;
				}
				DestroyWindow(Window);
				Window = nullptr;
				UnregisterClassW(L"OGL", hInstance);
			}
		}

		// Note: currently there is no way to get GL to use the passed pLuid
		bool InitDevice(int vpW, int vpH, const LUID*, bool windowed = true)
		{
			UNREFERENCED_PARAMETER(windowed);

			WinSizeW = vpW;
			WinSizeH = vpH;

			RECT size = { 0, 0, vpW, vpH };
			AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
			const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
			if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
				return false;

			PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
			{
				// First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
				PIXELFORMATDESCRIPTOR pfd;
				memset(&pfd, 0, sizeof(pfd));
				pfd.nSize = sizeof(pfd);
				pfd.nVersion = 1;
				pfd.iPixelType = PFD_TYPE_RGBA;
				pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
				pfd.cColorBits = 32;
				pfd.cDepthBits = 16;
				int pf = ChoosePixelFormat(hDC, &pfd);
				if (!pf) {
					log(Warning, "Failed to choose pixel format.");
					ReleaseDC(Window, hDC);
					return false;
				}

				if (!SetPixelFormat(hDC, pf, &pfd)) {
					log(Warning, "Failed to set pixel format.");
					ReleaseDC(Window, hDC);
					return false;
				}

				HGLRC context = wglCreateContext(hDC);
				if (!context) {
					log(Warning, "wglCreateContextfailed.");
					ReleaseDC(Window, hDC);
					return false;
				}
				if (!wglMakeCurrent(hDC, context)) {
					log(Warning, "wglMakeCurrent failed.");
					wglDeleteContext(context);
					ReleaseDC(Window, hDC);
					return false;
				}

				wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
				wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
				assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

				wglDeleteContext(context);
			}

			// Now create the real context that we will be using.
			int iAttributes[] =
			{
				// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 16,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
				0, 0
			};

			float fAttributes[] = { 0, 0 };
			int   pf = 0;
			UINT  numFormats = 0;

			if (!wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats)) {
				log(Warning, "wglChoosePixelFormatARBFunc failed.");
				ReleaseDC(Window, hDC);
				return false;
			}

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			if (!SetPixelFormat(hDC, pf, &pfd)) {
				log(Warning, "SetPixelFormat failed.");
				ReleaseDC(Window, hDC);
				return false;
			}

			GLint attribs[16];
			int   attribCount = 0;
			attribs[attribCount] = 0;

			WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
			if (!wglMakeCurrent(hDC, WglContext)) {
				log(Warning, "wglMakeCurrent failed.");
				wglDeleteContext(WglContext);
				ReleaseDC(Window, hDC);
				return false;
			}

			OVR::GLEContext::SetCurrentContext(&GLEContext);
			GLEContext.Init();

			glGenFramebuffers(1, &fboId);

			glEnable(GL_DEPTH_TEST);
			glFrontFace(GL_CW);
			glEnable(GL_CULL_FACE);

			return true;
		}

		bool HandleMessages(void)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return Running;
		}

		void Run(bool(*MainLoop)(bool retryCreate))
		{
			while (HandleMessages())
			{
				// true => we'll attempt to retry for ovrError_DisplayLost
				if (!MainLoop(true))
					break;
				// Sleep a bit before retrying to reduce CPU load while the HMD is disconnected
				Sleep(10);
			}
		}

		void ReleaseDevice()
		{
			if (fboId)
			{
				glDeleteFramebuffers(1, &fboId);
				fboId = 0;
			}
			if (WglContext)
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(WglContext);
				WglContext = nullptr;
			}
			GLEContext.Shutdown();
		}

	};

	// Global OpenGL state
	static OGL Platform;

	//------------------------------------------------------------------------------

	namespace VrInterface {

		// return true to retry later (e.g. after display lost)
		static bool MainLoop(bool retryCreate)
		{
			ovrMirrorTexture mirrorTexture = nullptr;
			GLuint mirrorFBO = 0;

			ovrSession session;
			ovrGraphicsLuid luid;
			ovrResult result = ovr_Create(&session, &luid);
			if (!OVR_SUCCESS(result)) {
				log(Info, "HMD not connected.");
				return retryCreate;
			}

			ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);

			// Setup Window and Graphics
			// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
			ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
			if (!Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid)))
				goto Done;
		
			// Make eye render buffers
			for (int eye = 0; eye < 2; ++eye)
			{
				ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
			}
		
			ovrMirrorTextureDesc desc;
			memset(&desc, 0, sizeof(desc));
			desc.Width = windowSize.w;
			desc.Height = windowSize.h;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

			// Create mirror texture and an FBO used to copy mirror texture to back buffer
			result = ovr_CreateMirrorTextureGL(session, &desc, &mirrorTexture);
			if (!OVR_SUCCESS(result))
			{
				if (retryCreate) goto Done;
				log(Warning, "Failed to create mirror texture.");
			}

			// Configure the mirror read buffer
			GLuint texId;
			ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);

			glGenFramebuffers(1, &mirrorFBO);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
			glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

			// Turn off vsync to let the compositor do its magic
			wglSwapIntervalEXT(0);

			// FloorLevel will give tracking poses where the floor height is 0
			ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
		
			// Main loop
			while (Platform.HandleMessages())
			{
				ovrSessionStatus sessionStatus;
				ovr_GetSessionStatus(session, &sessionStatus);
				if (sessionStatus.ShouldQuit)
				{
					// Because the application is requested to quit, should not request retry
					retryCreate = false;
					break;
				}
				if (sessionStatus.ShouldRecenter)
					ovr_RecenterTrackingOrigin(session);
				
			}
		
		Done:
			Platform.ReleaseDevice();
			ovr_Destroy(session);

			// Retry on ovrError_DisplayLost*/
			return retryCreate || (result == ovrError_DisplayLost);
		}

		void* Init(void* hinst) {
			ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
			ovrResult result = ovr_Initialize(&initParams);
			if (!OVR_SUCCESS(result)) {
				log(Warning, "Failed to initialize libOVR.");
				return(0);
			}

			if (!Platform.InitWindow((HINSTANCE)hinst, L"ORT(OpenGL)")) {
				log(Warning, "Failed to open window.");
				return(0);
			}

			Platform.Run(MainLoop);

			ovr_Shutdown();

			return Platform.Window;
		}
	}
}
#endif