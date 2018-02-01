#include "pch.h"

#ifdef KORE_OCULUS

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics3/Graphics.h>
#include <Kore/Log.h>

#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"
#include <assert.h>

using namespace Kore;

namespace {
	SensorState sensorStates[2];
}

struct TextureBuffer {
	ovrSession Session;
	ovrTextureSwapChain TextureChain;
	OVR::Sizei texSize;

	Graphics4::RenderTarget* OVRRenderTarget;

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

					OVRRenderTarget = new Graphics4::RenderTarget(texSize.w, texSize.h, 1);
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

		if (OVRRenderTarget) Graphics4::setRenderTarget(OVRRenderTarget);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, OVRRenderTarget->_depthTexture, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_FRAMEBUFFER_SRGB); // TODO: too bright
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

	OGL() : Window(nullptr), hDC(nullptr), WglContext(nullptr),GLEContext(), Running(false), WinSizeW(0), WinSizeH(0), hInstance(nullptr) {
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~OGL() {
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hInst, const char* title, const char* windowClassName) {
		hInstance = hInst;
		Running = true;

		// Adjust the window size and show at InitDevice time
		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, title, -1, wchTitle, 256);
		wchar_t wchClassName[256];
		MultiByteToWideChar(CP_ACP, 0, windowClassName, -1, wchClassName, 256);
		Window = CreateWindowW(wchClassName, wchTitle, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInst, 0);
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
			Window = nullptr;
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

		glEnable(GL_DEPTH_TEST);
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

	ovrPosef EyeRenderPose[2];
	double sensorSampleTime;

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

		ovrMirrorTextureDesc desc;
		memset(&desc, 0, sizeof(desc));
		desc.Width = Platform.WinSizeW;
		desc.Height = Platform.WinSizeH;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

		// Create mirror texture and an FBO used to copy mirror texture to back buffer
		ovrResult result = ovr_CreateMirrorTextureWithOptionsGL(session, &desc, &mirrorTexture);
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
	}
}

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName) {
	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		log(Warning, "Failed to initialize libOVR.");
		return(0);
	}
	
	if (!Platform.InitWindow((HINSTANCE)hinst, title, windowClassName)) {
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

	// Get both eye poses simultaneously, with IPD offset already included.
	ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);
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

namespace {

	mat4 convert(OVR::Matrix4f& m) {
		mat4 mat;
		mat.Set(0, 0, m.M[0][0]); mat.Set(0, 1, m.M[0][1]); mat.Set(0, 2, m.M[0][2]); mat.Set(0, 3, m.M[0][3]);
		mat.Set(1, 0, m.M[1][0]); mat.Set(1, 1, m.M[1][1]); mat.Set(1, 2, m.M[1][2]); mat.Set(1, 3, m.M[1][3]);
		mat.Set(2, 0, m.M[2][0]); mat.Set(2, 1, m.M[2][1]); mat.Set(2, 2, m.M[2][2]); mat.Set(2, 3, m.M[2][3]);
		mat.Set(3, 0, m.M[3][0]); mat.Set(3, 1, m.M[3][1]); mat.Set(3, 2, m.M[3][2]); mat.Set(3, 3, m.M[3][3]);
		return mat;
	}
}

SensorState VrInterface::getSensorState(int eye) {
	VrPoseState poseState;

	ovrQuatf orientation = EyeRenderPose[eye].Orientation;
	poseState.vrPose.orientation = Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);

	ovrVector3f pos = EyeRenderPose[eye].Position;
	poseState.vrPose.position = vec3(pos.x, pos.y, pos.z);

	ovrFovPort fov = hmdDesc.DefaultEyeFov[eye];
	poseState.vrPose.left = fov.LeftTan;
	poseState.vrPose.right = fov.RightTan;
	poseState.vrPose.bottom = fov.DownTan;
	poseState.vrPose.top = fov.UpTan;

	// Get view and projection matrices
	OVR::Matrix4f finalRollPitchYaw = OVR::Matrix4f(EyeRenderPose[eye].Orientation);
	OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
	OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
	OVR::Vector3f shiftedEyePos = EyeRenderPose[eye].Position;

	OVR::Matrix4f view = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
	OVR::Matrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);

	poseState.vrPose.eye = convert(view);
	poseState.vrPose.projection = convert(proj);

	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(session, &sessionStatus);
	if (sessionStatus.IsVisible) poseState.isVisible = true;
	else poseState.isVisible = false;
	if (sessionStatus.HmdPresent) poseState.hmdPresenting = true;
	else poseState.hmdPresenting = false;
	if (sessionStatus.HmdMounted) poseState.hmdMounted = true;
	else poseState.hmdMounted = false;
	if (sessionStatus.DisplayLost) poseState.displayLost = true;
	else poseState.displayLost = false;
	if (sessionStatus.ShouldQuit) poseState.shouldQuit = true;
	else poseState.shouldQuit = false;
	if (sessionStatus.ShouldRecenter) poseState.shouldRecenter = true;
	else poseState.shouldRecenter = false;

	sensorStates[eye].pose = poseState;

	return sensorStates[eye];
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
			ld.RenderPose[eye] = EyeRenderPose[eye];
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