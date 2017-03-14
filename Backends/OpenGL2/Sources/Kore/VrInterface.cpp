#include "pch.h"

#include <Kore/Vr/VrInterface.h>

#ifdef VR_RIFT

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>

using namespace Kore;
struct TextureBuffer {
	ovrSession Session;
	ovrTextureSwapChain TextureChain;
	OVR::Sizei texSize;

	RenderTarget* OVRRenderTarget;

	TextureBuffer(ovrSession session, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount) :
		Session(session), TextureChain(nullptr), texSize(size), OVRRenderTarget(nullptr) {
		UNREFERENCED_PARAMETER(sampleCount);

		assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

		if (displayableOnHmd) {
			// This texture isn't necessarily going to be a rendertarget, but it usually is.
			assert(session); // No HMD? A little odd.
			assert(sampleCount == 1); // ovr_CreateSwapTextureSetD3D11 doesn't support MSAA.

			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.SampleCount = 1;
			desc.StaticImage = ovrFalse;

			ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &TextureChain);

			int length = 0;
			ovr_GetTextureSwapChainLength(session, TextureChain, &length);

			if (OVR_SUCCESS(result)) {
				for (int i = 0; i < length; ++i) {
					GLuint chainTexId;
					ovr_GetTextureSwapChainBufferGL(Session, TextureChain, i, &chainTexId);
					glBindTexture(GL_TEXTURE_2D, chainTexId);

					OVRRenderTarget = new RenderTarget(texSize.w, texSize.h, 1);
				}
			}
		}

		if (mipLevels > 1) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	~TextureBuffer() {
		if (TextureChain) {
			ovr_DestroyTextureSwapChain(Session, TextureChain);
			TextureChain = nullptr;
		}
		if (OVRRenderTarget) {
			delete OVRRenderTarget;
			OVRRenderTarget = nullptr;
		}
	}

	OVR::Sizei GetSize() const {
		return texSize;
	}

	void SetAndClearRenderSurface() {
		GLuint curTexId;
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(Session, TextureChain, curIndex, &curTexId);

		if (OVRRenderTarget) Graphics::setRenderTarget(OVRRenderTarget, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, OVRRenderTarget->_depthTexture, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	void UnsetRenderSurface() {
		glBindFramebuffer(GL_FRAMEBUFFER, OVRRenderTarget->_framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}

	void Commit() {
		if (TextureChain) {
			ovr_CommitTextureSwapChain(Session, TextureChain);
		}
	}
};

//-------------------------------------------------------------------------------------------
struct OGL {
	HWND Window;
	HDC hDC;
	HGLRC WglContext;
	OVR::GLEContext GLEContext;
	bool Running;
	bool Key[256];
	int WinSizeW;
	int WinSizeH;
	HINSTANCE hInstance;

	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
		OGL *p = reinterpret_cast<OGL *>(GetWindowLongPtr(hWnd, 0));
		switch (Msg) {
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
		if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE]) {
			p->Running = false;
		}
		return 0;
	}

	OGL() : Window(nullptr), hDC(nullptr), WglContext(nullptr),GLEContext(), Running(false), WinSizeW(0), WinSizeH(0), hInstance(nullptr) {
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~OGL() {
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hInst, const char* title) {
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
		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, title, -1, wchTitle, 256);
		Window = CreateWindowW(wc.lpszClassName, wchTitle, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
		if (!Window) return false;

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		hDC = GetDC(Window);

		return true;
	}

	void CloseWindow() {
		if (Window) {
			if (hDC) {
				ReleaseDC(Window, hDC);
				hDC = nullptr;
			}
			DestroyWindow(Window);
			Window = nullptr;
			UnregisterClassW(L"OGL", hInstance);
		}
	}

	// Note: currently there is no way to get GL to use the passed pLuid
	bool InitDevice(int vpW, int vpH, const LUID* /*pLuid*/, bool windowed = true) {
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
		int iAttributes[] = {
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

		Graphics::setRenderState(RenderState::DepthTest, true);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);

		return true;
	}

	void ReleaseDevice() {
		if (WglContext) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(WglContext);
			WglContext = nullptr;
		}
		GLEContext.Shutdown();
	}
};

namespace {
	TextureBuffer* eyeRenderTexture[2] = { nullptr, nullptr };

	ovrMirrorTexture mirrorTexture = nullptr;
	uint mirrorFBO = 0;
	long long frameIndex = 0;
	bool isVisible = true;

	ovrSession session;
	ovrHmdDesc hmdDesc;

	ovrPosef EyePose[2];
	ovrPosef EyePredictedPose[2];
	double sensorSampleTime;
	double predictedFrameTiming;
	ovrTrackingState trackingState;

	OGL Platform;

	void done() {
		if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
		if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
		for (int eye = 0; eye < 2; ++eye) {
			delete eyeRenderTexture[eye];
		}
		Platform.ReleaseDevice();
		ovr_Destroy(session);
	}

	void createOculusTexture() {
		// Make eye render buffers
		for (int eye = 0; eye < 2; ++eye) {
			ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
			eyeRenderTexture[eye] = new TextureBuffer(session, true, idealTextureSize, 1, NULL, 1);

			if (!eyeRenderTexture[eye]->TextureChain) {
				log(Info, "Failed to create texture.");
				done();
			}
		}
	}
}

void* VrInterface::init(void* hinst, const char* title) {
	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		log(Warning, "Failed to initialize libOVR.");
		return(0);
	}

	if (!Platform.InitWindow((HINSTANCE)hinst, title)) {
		log(Warning, "Failed to open window.");
		return(0);
	}

	ovrGraphicsLuid luid;
	result = ovr_Create(&session, &luid);
	if (!OVR_SUCCESS(result)) {
		log(Info, "HMD not connected.");
		return false; // TODO: retry
	}

	hmdDesc = ovr_GetHmdDesc(session);

	// Setup Window and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
	if (!Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid))) {
		log(Info, "Failed to init device.");
		done();
	}

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = Platform.WinSizeW;
	desc.Height = Platform.WinSizeH;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

	// Create mirror texture and an FBO used to copy mirror texture to back buffer
	result = ovr_CreateMirrorTextureGL(session, &desc, &mirrorTexture);
	if (!OVR_SUCCESS(result)) {
		log(Info, "Failed to create mirror texture.");
		done();
	}

	// Configure the mirror read buffer
	GLuint texId;
	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &texId);

	glGenFramebuffers(1, &mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	// FloorLevel will give tracking poses where the floor height is 0
	ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);

	// Return window
	return Platform.Window;
}

void VrInterface::begin() {
	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyeOffset) may change at runtime.
	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f HmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset, eyeRenderDesc[1].HmdToEyeOffset };

	// Get predicted eye pose
	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyePose, &sensorSampleTime);

	// Ask the API for the times when this frame is expected to be displayed. 
	predictedFrameTiming = ovr_GetPredictedDisplayTime(session, frameIndex);
	trackingState = ovr_GetTrackingState(session, predictedFrameTiming, ovrTrue);
	ovr_CalcEyePoses(trackingState.HeadPose.ThePose, HmdToEyeOffset, EyePredictedPose);
}

void VrInterface::beginRender(int eye) {
	if (eyeRenderTexture[0] == nullptr || eyeRenderTexture[1] == nullptr) createOculusTexture();
	// Switch to eye render target
	eyeRenderTexture[eye]->SetAndClearRenderSurface();
}

void VrInterface::endRender(int eye) {
	// Avoids an error when calling SetAndClearRenderSurface during next iteration.
	eyeRenderTexture[eye]->UnsetRenderSurface();
	// Commit changes to the textures so they get picked up frame
	eyeRenderTexture[eye]->Commit();
}

SensorState* VrInterface::getSensorState(int eye) {
	SensorState* sensorState = new SensorState();
	VrPoseState* poseState = new VrPoseState();

	ovrQuatf orientation = EyePose[eye].Orientation;
	poseState->vrPose->orientation = Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);

	ovrVector3f pos = EyePose[eye].Position;
	poseState->vrPose->position = vec3(pos.x, pos.y, pos.z);

	ovrFovPort fov = hmdDesc.DefaultEyeFov[eye];
	poseState->vrPose->left = fov.LeftTan;
	poseState->vrPose->right = fov.RightTan;
	poseState->vrPose->bottom = fov.DownTan;
	poseState->vrPose->top = fov.UpTan;

	ovrVector3f angularVelocity = trackingState.HeadPose.AngularVelocity;
	ovrVector3f linearVelocity = trackingState.HeadPose.LinearVelocity;
	ovrVector3f angularAcceleration = trackingState.HeadPose.AngularAcceleration;
	ovrVector3f linearAcceleration = trackingState.HeadPose.LinearAcceleration;
	poseState->angularVelocity = vec3(angularVelocity.x, angularVelocity.y, angularVelocity.z);
	poseState->linearVelocity = vec3(linearVelocity.x, linearVelocity.y, linearVelocity.z);
	poseState->angularAcceleration = vec3(angularAcceleration.x, angularAcceleration.y, angularAcceleration.z);
	poseState->linearAcceleration = vec3(linearAcceleration.x, linearAcceleration.y, linearAcceleration.z);

	// Get predicted orientation and position
	VrPoseState* predictedPoseState = new VrPoseState();
	ovrQuatf predOrientation = EyePredictedPose[eye].Orientation;
	predictedPoseState->vrPose->orientation = Quaternion(predOrientation.x, predOrientation.y, predOrientation.z, predOrientation.w);

	ovrVector3f predPos = EyePredictedPose[eye].Position;
	predictedPoseState->vrPose->position = vec3(predPos.x, predPos.y, predPos.z);

	//sensorState->pose = poseState;
	sensorState->pose = predictedPoseState;

	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(session, &sessionStatus);
	if (sessionStatus.IsVisible) sensorState->isVisible = true;
	else sensorState->isVisible = false;
	if (sessionStatus.HmdPresent) sensorState->hmdPresenting = true;
	else sensorState->hmdPresenting = false;
	if (sessionStatus.HmdMounted) sensorState->hmdMounted = true;
	else sensorState->hmdMounted = false;
	if (sessionStatus.DisplayLost) sensorState->displayLost = true;
	else sensorState->displayLost = false;
	if (sessionStatus.ShouldQuit) sensorState->shouldQuit = true;
	else sensorState->shouldQuit = false;
	if (sessionStatus.ShouldRecenter) sensorState->shouldRecenter = true;
	else sensorState->shouldRecenter = false;

	return sensorState;
}

void VrInterface::warpSwap() {
	// Initialize our single full screen Fov layer.
	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	if (isVisible) {
		for (int eye = 0; eye < 2; ++eye) {
			ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureChain;
			ld.Viewport[eye] = OVR::Recti(eyeRenderTexture[eye]->GetSize());
			ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = EyePose[eye];
			ld.SensorSampleTime = sensorSampleTime;
		}
	}

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	if (!OVR_SUCCESS(result)) {
		isVisible = false;
	} else {
		isVisible = true;
	}

	frameIndex++;

	// Blit mirror texture to back buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GLint w = Platform.WinSizeW;
	GLint h = Platform.WinSizeH;
	glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin) {
	switch (origin) {
	case Stand:
		ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
		break;
	case Sit:
		ovr_SetTrackingOriginType(session, ovrTrackingOrigin_EyeLevel);
		break;
	default:
		ovr_SetTrackingOriginType(session, ovrTrackingOrigin_FloorLevel);
		break;
	}
}

void VrInterface::resetHmdPose() {
	ovr_RecenterTrackingOrigin(session);
}

void VrInterface::ovrShutdown() {
	ovr_Shutdown();
}

#endif