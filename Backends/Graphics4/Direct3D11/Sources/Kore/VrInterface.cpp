#include "pch.h"

#ifdef VR_RIFT

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include "Direct3D11.h"

#include "OVR_CAPI_D3D.h"

#include <vector>
#include "d3d11.h"
#if _MSC_VER > 1600
#include "DirectXMath.h"
using namespace DirectX;
#else
#include "xnamath.h"
#endif //_MSC_VER > 1600

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Kore;

//------------------------------------------------------------
struct DepthBuffer {
	ID3D11DepthStencilView* TexDsv;

	DepthBuffer(ID3D11Device * Device, int sizeW, int sizeH, int sampleCount = 1) {
		DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT;
		D3D11_TEXTURE2D_DESC dsDesc;
		dsDesc.Width = sizeW;
		dsDesc.Height = sizeH;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.Format = format;
		dsDesc.SampleDesc.Count = sampleCount;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags = 0;
		dsDesc.MiscFlags = 0;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ID3D11Texture2D * Tex;
		Device->CreateTexture2D(&dsDesc, NULL, &Tex);
		Device->CreateDepthStencilView(Tex, NULL, &TexDsv);
		Tex->Release();
	}
	~DepthBuffer() {
		TexDsv->Release();
		TexDsv = nullptr;
	}
};

//---------------------------------------------------------------------
struct DirectX11 {
	HWND Window;
	bool Running;
	bool Key[256];
	int WinSizeW;
	int WinSizeH;

	HINSTANCE hInstance;

	DirectX11() : Window(nullptr), Running(false), WinSizeW(0), WinSizeH(0), hInstance(nullptr) {
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~DirectX11() {
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hinst, const char* title, const char* windowClassName) {
		hInstance = hinst;
		Running = true;

		// Adjust the window size and show at InitDevice time
		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, title, -1, wchTitle, 256);
		wchar_t wchClassName[256];
		MultiByteToWideChar(CP_ACP, 0, windowClassName, -1, wchClassName, 256);
		Window = CreateWindowW(wchClassName, wchTitle, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hinst, 0);
		if (!Window) return false;

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		return true;
	}

	void CloseWindow() {
		if (Window) {
			Window = nullptr;
		}
	}

	bool InitDevice(int vpW, int vpH, const LUID* pLuid, bool windowed = true, int scale = 1) {
		WinSizeW = vpW;
		WinSizeH = vpH;

		if (scale == 0)
			scale = 1;

		RECT size = { 0, 0, vpW / scale, vpH / scale };
		AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
		const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
		if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
			return false;

		return true;
	}

	void SetAndClearRenderTarget(ID3D11RenderTargetView* rendertarget, DepthBuffer* depthbuffer,
								 float R = 0, float G = 0, float B = 0, float A = 0) {
		float black[] = { R, G, B, A }; // Important that alpha=0, if want pixels to be transparent, for manual layers
		context->OMSetRenderTargets(1, &rendertarget, (depthbuffer ? depthbuffer->TexDsv : nullptr));
		context->ClearRenderTargetView(rendertarget, black);
		if (depthbuffer)
			context->ClearDepthStencilView(depthbuffer->TexDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	void SetViewport(float vpX, float vpY, float vpW, float vpH) {
		D3D11_VIEWPORT D3Dvp;
		D3Dvp.Width = vpW;    D3Dvp.Height = vpH;
		D3Dvp.MinDepth = 0;   D3Dvp.MaxDepth = 1;
		D3Dvp.TopLeftX = vpX; D3Dvp.TopLeftY = vpY;
		context->RSSetViewports(1, &D3Dvp);
	}

	void ReleaseDevice() {
	}
};

static DirectX11 Platform;

//---------------------------------------------------------------------

// ovrSwapTextureSet wrapper class that also maintains the render target views needed for D3D11 rendering.
struct OculusTexture {
	ovrSession Session;
	ovrTextureSwapChain TextureChain;
	std::vector<ID3D11RenderTargetView*> TexRtv;

	OculusTexture(ovrSession session, int sizeW, int sizeH) : Session(session), TextureChain(nullptr) {
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = 1;
		desc.MiscFlags = ovrTextureMisc_DX_Typeless;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;
		desc.StaticImage = ovrFalse;

		ovrResult result = ovr_CreateTextureSwapChainDX(session, device, &desc, &TextureChain);

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(Session, TextureChain, &textureCount);
		if (OVR_SUCCESS(result)) {
			for (int i = 0; i < textureCount; ++i) {
				ID3D11Texture2D* tex = nullptr;
				ovr_GetTextureSwapChainBufferDX(Session, TextureChain, i, IID_PPV_ARGS(&tex));
				D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
				rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				ID3D11RenderTargetView* rtv;
				device->CreateRenderTargetView(tex, &rtvd, &rtv);
				TexRtv.push_back(rtv);
				tex->Release();
			}
		}
	}

	~OculusTexture() {
		for (int i = 0; i < (int)TexRtv.size(); ++i) {
			TexRtv[i]->Release();
			TexRtv[i] = nullptr;
		}
		if (TextureChain) {
			ovr_DestroyTextureSwapChain(Session, TextureChain);
			TextureChain = nullptr;
		}
	}

	ID3D11RenderTargetView* GetRTV() {
		int index = 0;
		ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &index);
		return TexRtv[index];
	}

	void Commit() {
		ovr_CommitTextureSwapChain(Session, TextureChain);
	}
};
//---------------------------------------------------------------------

namespace {
	ovrRecti eyeRenderViewport[2];
	OculusTexture* pEyeRenderTexture[2] = { nullptr, nullptr };
	DepthBuffer* pEyeDepthBuffer[2] = { nullptr, nullptr };

	ovrSizei windowSize;

	ovrMirrorTexture mirrorTexture = nullptr;
	long long frameIndex = 0;
	bool isVisible = true;

	ovrSession session;
	ovrHmdDesc hmdDesc;

	ovrPosef EyePose[2];
	ovrPosef EyePredictedPose[2];
	double sensorSampleTime;
	double predictedFrameTiming;
	ovrTrackingState trackingState;

	void done() {
		if (mirrorTexture)
			ovr_DestroyMirrorTexture(session, mirrorTexture);
		for (int eye = 0; eye < 2; ++eye) {
			delete pEyeRenderTexture[eye];
			delete pEyeDepthBuffer[eye];
		}
		Platform.ReleaseDevice();
		ovr_Destroy(session);
	}

	void createOculusTexture() {
		// Create mirror texture
		ovrMirrorTextureDesc desc;
		memset(&desc, 0, sizeof(desc));
		desc.Width = Platform.WinSizeW;
		desc.Height = Platform.WinSizeH;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		HRESULT result = ovr_CreateMirrorTextureDX(session, device, &desc, &mirrorTexture);
		if (!OVR_SUCCESS(result)) {
			log(Error, "Failed to create mirror texture.");
			done();
		}

		// Make eye render buffers
		for (int eye = 0; eye < 2; ++eye) {
			ovrSizei idealSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
			pEyeRenderTexture[eye] = new OculusTexture(session, idealSize.w, idealSize.h);
			pEyeDepthBuffer[eye] = new DepthBuffer(device, idealSize.w, idealSize.h);
			eyeRenderViewport[eye].Pos.x = 0;
			eyeRenderViewport[eye].Pos.y = 0;
			eyeRenderViewport[eye].Size = idealSize;
			if (!pEyeRenderTexture[eye]->TextureChain) {
				log(Error, "Failed to create texture.");
				done();
			}
		}
	}
}

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName) {
	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		log(Error, "Failed to initialize libOVR.");
		return(0);
	}

	if (!Platform.InitWindow((HINSTANCE)hinst, title, windowClassName)) {
		log(Error, "Failed to open window.");
		return(0);
	}

	ovrGraphicsLuid luid;
	result = ovr_Create(&session, &luid);
	if (!OVR_SUCCESS(result)) {
		log(Error, "HMD not connected.");
		return false; // TODO: retry
	}

	hmdDesc = ovr_GetHmdDesc(session);

	// Setup Window and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
	if (!Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid))) {
		log(Error, "Failed to init device.");
		done();
	}

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
	if (pEyeRenderTexture[0] == nullptr || pEyeRenderTexture[1] == nullptr) createOculusTexture();

	// Clear and set up rendertarget
	Platform.SetAndClearRenderTarget(pEyeRenderTexture[eye]->GetRTV(), pEyeDepthBuffer[eye]);
	Platform.SetViewport((float)eyeRenderViewport[eye].Pos.x, (float)eyeRenderViewport[eye].Pos.y,
						 (float)eyeRenderViewport[eye].Size.w, (float)eyeRenderViewport[eye].Size.h);
}

void VrInterface::endRender(int eye) {
	// Commit rendering to the swap chain
	pEyeRenderTexture[eye]->Commit();
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

	sensorState->predictedPose = predictedPoseState;
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
	// Initialize our single full screen Fov layer.
	ovrLayerEyeFov ld = {};
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = 0; 

	if (isVisible) {
		for (int eye = 0; eye < 2; ++eye) {
			ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureChain;
			ld.Viewport[eye] = eyeRenderViewport[eye];
			ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = EyePose[eye];		// eyePredictedPose[eye];
			ld.SensorSampleTime = sensorSampleTime;		// predictedFrameTiming;
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

	// Render mirror
	ID3D11Texture2D* tex = nullptr;
	ovr_GetMirrorTextureBufferDX(session, mirrorTexture, IID_PPV_ARGS(&tex));
	context->CopyResource(backBuffer, tex);
	tex->Release();
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