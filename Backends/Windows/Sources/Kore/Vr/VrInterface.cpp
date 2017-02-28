#include "pch.h"

#include "VrInterface.h"

#ifdef VR_RIFT

#include "Kore/Log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <assert.h>

using namespace Kore;
//---------------------------------------------------------------------------------------
struct DepthBuffer {
	GLuint        texId;

	DepthBuffer(OVR::Sizei size, int sampleCount)
	{
		UNREFERENCED_PARAMETER(sampleCount);

		assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum internalFormat = GL_DEPTH_COMPONENT24;
		GLenum type = GL_UNSIGNED_INT;
		if (GLE_ARB_depth_buffer_float) {
			internalFormat = GL_DEPTH_COMPONENT32F;
			type = GL_FLOAT;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
	}
	~DepthBuffer()
	{
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
	}
};

OculusTexture::OculusTexture(ovrSession session, bool displayableOnHmd, OVR::Sizei size, int mipLevels, unsigned char * data, int sampleCount) :
	session(session), textureChain(nullptr), texSize(size), texId(0), fboId(0), renderTarget(nullptr) {

	UNREFERENCED_PARAMETER(sampleCount);

	assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

	if (displayableOnHmd) {
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

		ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &textureChain);

		int length = 0;
		ovr_GetTextureSwapChainLength(session, textureChain, &length);

		if (OVR_SUCCESS(result)) {
			for (int i = 0; i < length; ++i) {
				GLuint chainTexId;
				ovr_GetTextureSwapChainBufferGL(session, textureChain, i, &chainTexId);
				glBindTexture(GL_TEXTURE_2D, chainTexId);

				// TODO: depthBufferBits?
				//renderTarget = new RenderTarget(texSize.w, texSize.h, 0);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	if (mipLevels > 1) {
		// TODO set
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glGenFramebuffers(1, &fboId);
}

OculusTexture::~OculusTexture() {
	if (textureChain) {
		ovr_DestroyTextureSwapChain(session, textureChain);
		textureChain = nullptr;
	}
	if (texId) {
		glDeleteTextures(1, &texId);
		texId = 0;
	}
	if (fboId) {
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}
	if (renderTarget) {
		renderTarget = nullptr;
	}
}

OVR::Sizei OculusTexture::getSize() const {
	return texSize;
}

void OculusTexture::setAndClearRenderSurface(int texId) {
	GLuint curTexId;
	if (textureChain) {
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(session, textureChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(session, textureChain, curIndex, &curTexId);
	}
	else {
		curTexId = texId;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);

	glViewport(0, 0, texSize.w, texSize.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_FRAMEBUFFER_SRGB);

	if (renderTarget)
		Graphics::setRenderTarget(renderTarget, 0, 0);
}

void OculusTexture::unsetRenderSurface() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void OculusTexture::commit() {
	if (textureChain) {
		ovr_CommitTextureSwapChain(session, textureChain);
	}
}

ovrTextureSwapChain OculusTexture::getOculusTexture() {
	return textureChain;
}

//-------------------------------------------------------------------------------------------

ovrSession session;
ovrHmdDesc hmdDesc;
long frameIndex;
ovrPosef EyeRenderPose[2];
double sensorSampleTime;

OculusTexture* eyeRenderTexture[2] = { nullptr, nullptr };
DepthBuffer  * eyeDepthBuffer[2] = { nullptr, nullptr };
bool isVisible = true;

ovrMirrorTexture mirrorTexture = nullptr;
GLuint          mirrorFBO = 0;

struct OGL {
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

	OGL() :
		Window(nullptr),
		hDC(nullptr),
		WglContext(nullptr),
		GLEContext(),
		Running(false),
		WinSizeW(0),
		WinSizeH(0),
		fboId(0),
		hInstance(nullptr) {
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~OGL() {
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindowAndDevice(HINSTANCE hInst, LPCWSTR title) {
		// Init window
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
		Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
		//Window = CreateWindowA("ORT", "ORT(OpenGL)", WS_POPUP, 0, 0, 1000, 1000, GetDesktopWindow(), NULL, hInstance, NULL);
		if (!Window) {
			log(Info, "Failed to open window.");
			return false;
		}

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		hDC = GetDC(Window);

		// Init device
		ovrGraphicsLuid luid;
		ovrResult result = ovr_Create(&session, &luid);
		if (!OVR_SUCCESS(result)) {
			log(Info, "HMD not connected.");
			return false; // todo: retry
		}

		hmdDesc = ovr_GetHmdDesc(session);

		// Setup Window and Graphics
		// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
		ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
		if (!InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid))) {
			//Platform.releaseAndDestroy();
			log(Info, "Failed to init device.");
		}

		// Make eye render buffers
		for (int eye = 0; eye < 2; ++eye) {
			ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
			eyeRenderTexture[eye] = new OculusTexture(session, true, idealTextureSize, 1, NULL, 1);
			eyeDepthBuffer[eye] = new DepthBuffer(eyeRenderTexture[eye]->getSize(), 0);

			if (!eyeRenderTexture[eye]->getOculusTexture()) {
				ReleaseDevice();
				ovr_Destroy(session);
				log(Info, "Failed to create texture.");
			}
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
			ReleaseDevice();
			ovr_Destroy(session);
			log(Info, "Failed to create mirror texture.");
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
	bool InitDevice(int vpW, int vpH, const LUID*, bool windowed = true) {
		UNREFERENCED_PARAMETER(windowed);

		WinSizeW = vpW;
		WinSizeH = vpH;

		RECT size = { 0, 0, vpW, vpH };
		AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
		const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
		if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
			return false;

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr; {
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
			assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc); //OVR_ASSERT

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

		//ShowWindow(Window, SW_SHOWDEFAULT); // TODO: ???

		glGenFramebuffers(1, &fboId);

		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);

		//SetCapture(Platform.Window);	// TODO: ???
		//ShowCursor(FALSE);				// TODO: ???

		return true;
	}

	void ReleaseDevice() {
		if (fboId) {
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
		if (WglContext) {
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

namespace {
	void releaseAndDestroy() {
		if (mirrorFBO) glDeleteFramebuffers(1, &mirrorFBO);
		if (mirrorTexture) ovr_DestroyMirrorTexture(session, mirrorTexture);
		for (int eye = 0; eye < 2; ++eye) {
			delete eyeRenderTexture[eye];
			delete eyeDepthBuffer[eye];
		}
		Platform.ReleaseDevice();
		ovr_Destroy(session);
	}
}

void* VrInterface::init(void* hinst) {
	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		log(Warning, "Failed to initialize libOVR.");
		return(0);
	}

	if (!Platform.InitWindowAndDevice((HINSTANCE)hinst, L"ORT(OpenGL)")) {
		log(Warning, "Failed to init window and device.");
		return(0);
	}

	return Platform.Window;
}

void VrInterface::begin(int eye) {
	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyeOffset) may change at runtime.
	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	// Get eye poses, feeding in correct IPD offset
	ovrVector3f HmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset, eyeRenderDesc[1].HmdToEyeOffset };

	// Get predicted eye pose
	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime);

	// Switch to eye render target
	eyeRenderTexture[eye]->setAndClearRenderSurface(eyeDepthBuffer[eye]->texId);
}

void VrInterface::end(int eye) {
	eyeRenderTexture[eye]->unsetRenderSurface();
	// Commit changes to the textures so they get picked up frame
	eyeRenderTexture[eye]->commit();
}

SensorState* VrInterface::getSensorState(int eye) {
	SensorState* sensorState = new SensorState();
	VrPoseState* poseState = new VrPoseState();

	ovrQuatf orientation = EyeRenderPose[eye].Orientation;
	poseState->vrPose->orientation = Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);

	ovrVector3f pos = EyeRenderPose[eye].Position;
	poseState->vrPose->position = vec3(pos.x, pos.y, pos.z);

	ovrFovPort fov = hmdDesc.DefaultEyeFov[eye];
	poseState->vrPose->left = fov.LeftTan;
	poseState->vrPose->right = fov.RightTan;
	poseState->vrPose->bottom = fov.DownTan;
	poseState->vrPose->top = fov.UpTan;

	ovrTrackingState ts = ovr_GetTrackingState(session, 0.0, ovrFalse);
	ovrVector3f  angularVelocity = ts.HeadPose.AngularVelocity;
	ovrVector3f  linearVelocity = ts.HeadPose.LinearVelocity;
	ovrVector3f  angularAcceleration = ts.HeadPose.AngularAcceleration;
	ovrVector3f  linearAcceleration = ts.HeadPose.LinearAcceleration;
	poseState->angularVelocity = vec3(angularVelocity.x, angularVelocity.y, angularVelocity.z);
	poseState->linearVelocity = vec3(linearVelocity.x, linearVelocity.y, linearVelocity.z);
	poseState->angularAcceleration = vec3(angularAcceleration.x, angularAcceleration.y, angularAcceleration.z);
	poseState->linearAcceleration = vec3(linearAcceleration.x, linearAcceleration.y, linearAcceleration.z);
	sensorState->pose = poseState;

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
	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };

	if (isVisible) {
		for (int eye = 0; eye < 2; ++eye) {
			ld.ColorTexture[eye] = eyeRenderTexture[eye]->getOculusTexture();
			ld.Viewport[eye] = OVR::Recti(eyeRenderTexture[eye]->getSize());
			/*if (eye == 0) {
				ld.Viewport[0] = OVR::Recti(0, 0, windowSize.w / 2, windowSize.h);
			} else {
				ld.Viewport[1] = OVR::Recti(windowSize.w / 2, 0, windowSize.w / 2, windowSize.h);
			}*/
			ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = EyeRenderPose[eye];
			ld.SensorSampleTime = sensorSampleTime;
		}
	}

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	if (!OVR_SUCCESS(result)) {
		releaseAndDestroy();
		isVisible = false;
	}

	//frameIndex++;
	ovr_GetPredictedDisplayTime(session, frameIndex);

	// Blit mirror texture to back buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GLint w = windowSize.w;
	GLint h = windowSize.h;
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