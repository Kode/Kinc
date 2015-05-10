#include "pch.h"
#include "VrInterface.h"

#ifdef VR_RIFT

#include <GL/CAPI_GLE.h>
#include <OVR_CAPI_GL.h>


#include <Kernel/OVR_System.h>

#include <kha/vr/TimeWarpImage.h>
#include <kha/vr/PoseState.h>
#include <kha/vr/Pose.h>
#include <kha/math/Vector3.h>
#include <kha/math/Quaternion.h>


#include <Extras/OVR_Math.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

using namespace kha::vr;

namespace Kore {
//
namespace VrInterface {


	ovrHmd HMD;

	ovrGLConfig config;

	ovrEyeRenderDesc EyeRenderDesc[2];
	

	

	//-------------------------------------------------------------------------------------------
	struct OGL
	{
		HWND				Window;
		HDC					hDC;
		HGLRC				WglContext;
		OVR::GLEContext     GLEContext;

		GLuint				fboId;

		bool                Key[256];

		bool InitWindowAndDevice(HINSTANCE hInst, OVR::Recti vp, bool windowed, char * deviceName)
		{
			WglContext = 0;
			/* WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L,
				GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ORT", NULL };
			RegisterClassEx(&wc); */

			Window = CreateWindowA("ORT", "ORT(OpenGL)", WS_POPUP, vp.x, vp.y, vp.w, vp.h,
				GetDesktopWindow(), NULL, hInst, NULL);

			hDC = GetDC(Window);

			PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = NULL;
			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = NULL;
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
				if (!pf)
				{
					ReleaseDC(Window, hDC);
					return false;
				}

				if (!SetPixelFormat(hDC, pf, &pfd))
				{
					ReleaseDC(Window, hDC);
					return false;
				}

				HGLRC context = wglCreateContext(hDC);
				if (!wglMakeCurrent(hDC, context))
				{
					wglDeleteContext(context);
					ReleaseDC(Window, hDC);
					return false;
				}

				wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
				wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
				OVR_ASSERT(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

				wglDeleteContext(context);
			}

			// Now create the real context that we will be using.
			int iAttributes[] = {
				//WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 16,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
				0, 0 };

			float fAttributes[] = { 0, 0 };
			int   pf = 0;
			UINT  numFormats = 0;

			if (!wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats))
			{
				ReleaseDC(Window, hDC);
				return false;
			}

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));

			if (!SetPixelFormat(hDC, pf, &pfd))
			{
				ReleaseDC(Window, hDC);
				return false;
			}

			GLint attribs[16];
			int   attribCount = 0;
			int   flags = 0;
			int   profileFlags = 0;

			attribs[attribCount] = 0;

			WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
			if (!wglMakeCurrent(hDC, WglContext))
			{
				wglDeleteContext(WglContext);
				ReleaseDC(Window, hDC);
				return false;
			}

			OVR::GLEContext::SetCurrentContext(&GLEContext);
			GLEContext.Init();

			ShowWindow(Window, SW_SHOWDEFAULT);

			OVR::glGenFramebuffers(1, &fboId);

			glEnable(GL_DEPTH_TEST);
			glFrontFace(GL_CW);
			glEnable(GL_CULL_FACE);

			SetCapture(Platform.Window);

			ShowCursor(FALSE);

			return true;
		}

		void HandleMessages(void)
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				if (msg.message == WM_KEYDOWN) Key[msg.wParam] = true;
				if (msg.message == WM_KEYUP)   Key[msg.wParam] = false;
			}
		}

		void ReleaseWindow(HINSTANCE hInst)
		{
			ReleaseCapture();
			ShowCursor(TRUE);

			OVR::glDeleteFramebuffers(1, &fboId);

			if (WglContext) {
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(WglContext);
			}

			UnregisterClass(L"ORT", hInst);
		}

	} Platform;

	void* Init(void* hinst) {
		OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

		//Initialise rift
		if (!ovr_Initialize()) { MessageBoxA(NULL, "Unable to initialize libOVR.", "", MB_OK); return 0; }
		HMD = ovrHmd_Create(0);
		if (HMD == NULL)
		{
			HMD = ovrHmd_CreateDebug(ovrHmd_DK2);
		}

		if (!HMD) { MessageBoxA(NULL, "Oculus Rift not detected.", "", MB_OK); ovr_Shutdown(); return 0; }
		if (HMD->ProductName[0] == '\0') MessageBoxA(NULL, "Rift detected, display not enabled.", "", MB_OK);

		bool windowed = (HMD->HmdCaps & ovrHmdCap_ExtendDesktop) ? false : true;

		if (!Platform.InitWindowAndDevice((HINSTANCE) hinst, OVR::Recti(HMD->WindowsPos, HMD->Resolution), windowed, (char *)HMD->DisplayDeviceName))
			return 0;

		config.OGL.Header.API = ovrRenderAPI_OpenGL;
		config.OGL.Header.BackBufferSize = HMD->Resolution;
		config.OGL.Header.Multisample = 0;
		config.OGL.Window = Platform.Window;
		config.OGL.DC = Platform.hDC;


		ovrHmd_ConfigureRendering(HMD, &config.Config,
			ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp |
			ovrDistortionCap_Overdrive, HMD->DefaultEyeFov, EyeRenderDesc);

		ovrHmd_SetEnabledCaps(HMD, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
		ovrHmd_AttachToWindow(HMD, Platform.Window, NULL, NULL);

		// Start the sensor
		ovrHmd_ConfigureTracking(HMD, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection |
			ovrTrackingCap_Position, 0);

		// Dismiss the health and safety warning
		ovrHmd_DismissHSWDisplay(HMD);

		return Platform.Window;
	}

	template<typename T> T* CreateEmpty() {
	
		return dynamic_cast<T*>(T::__CreateEmpty().mPtr);
	}

	kha::vr::SensorState_obj* GetSensorState() {

		//Get eye poses, feeding in correct IPD offset
		ovrVector3f ViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset, EyeRenderDesc[1].HmdToEyeViewOffset };
		ovrPosef EyeRenderPose[2];
		ovrHmd_GetEyePoses(HMD, 0, ViewOffset, EyeRenderPose, NULL);

		
		kha::vr::SensorState_obj* state = dynamic_cast<kha::vr::SensorState_obj*>(kha::vr::SensorState_obj::__CreateEmpty().mPtr);
		
		state->Predicted = CreateEmpty<kha::vr::PoseState_obj>();
		state->Predicted->Pose = CreateEmpty<kha::vr::Pose_obj>();
		state->Predicted->Pose->Position = CreateEmpty<kha::math::Vector3_obj>();
		state->Predicted->Pose->Orientation = CreateEmpty<kha::math::Quaternion_obj>();
		state->Predicted->Pose->Orientation->__construct(0.0f, 0.0f, 0.0f, 0.0f);
		state->Predicted->Pose->Orientation->set_x(EyeRenderPose[0].Orientation.x);
		state->Predicted->Pose->Orientation->set_y(EyeRenderPose[0].Orientation.y);
		state->Predicted->Pose->Orientation->set_z(EyeRenderPose[0].Orientation.z);
		state->Predicted->Pose->Orientation->set_w(EyeRenderPose[0].Orientation.w);


			
				//	kha::vr::PoseState_obj* poseState = CreateEmpty<kha::vr::PoseState_obj>();
				//
				//	poseState->TimeInSeconds = nativeState.TimeInSeconds;
				//	poseState->AngularAcceleration = GetVector3(nativeState.AngularAcceleration);
				//	poseState->AngularVelocity = GetVector3(nativeState.AngularVelocity);
				//	poseState->LinearAcceleration = GetVector3(nativeState.LinearAcceleration);
				//	poseState->LinearVelocity = GetVector3(nativeState.LinearVelocity);
				//
				//	poseState->Pose = GetPose(nativeState.Pose);
				//
				//	return poseState;
				//}

		state->Recorded = state->Predicted;

		return state;
	}
	


	void WarpSwap(kha::vr::TimeWarpParms_obj* parms) {
		ovrHmd_BeginFrame(HMD, 0);

		ovrVector3f ViewOffset[2] = { EyeRenderDesc[0].HmdToEyeViewOffset, EyeRenderDesc[1].HmdToEyeViewOffset };
		ovrPosef EyeRenderPose[2];
		ovrHmd_GetEyePoses(HMD, 0, ViewOffset, EyeRenderPose, NULL);

		unsigned int leftImage = parms->LeftImage->Image->renderTarget->_texture;
		unsigned int rightImage = parms->RightImage->Image->renderTarget->_texture;
		
		ovrGLTexture eyeTex[2];

		// TODO: Should be set from the ideal size given by OVR
		ovrSizei size;
		size.w = 1182;
		size.h = 1464;

		for (int i = 0; i < 2; i++) {
			eyeTex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
			eyeTex[i].OGL.Header.TextureSize = size;
			eyeTex[i].OGL.Header.RenderViewport = OVR::Recti(OVR::Vector2i(0, 0), size);
		}

		
		eyeTex[0].OGL.TexId = leftImage;
		eyeTex[1].OGL.TexId = rightImage;
	
		ovrHmd_EndFrame(HMD, EyeRenderPose, &eyeTex[0].Texture);

		
	}
	

}

}

#endif
