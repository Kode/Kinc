#include "pch.h"

#ifdef KORE_OCULUS

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics4/Graphics.h>
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

namespace {
	SensorState sensorStates[2];
}

//------------------------------------------------------------
struct DepthBuffer {
	ID3D11DepthStencilView* TexDsv;

	DepthBuffer(ID3D11Device* Device, int sizeW, int sizeH, int sampleCount = 1) {
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
		Device->CreateTexture2D(&dsDesc, nullptr, &Tex);
		Device->CreateDepthStencilView(Tex, nullptr, &TexDsv);
		Tex->Release();
	}
	~DepthBuffer() {
		TexDsv->Release();
		TexDsv = nullptr;
	}
};

//-----------------------------------------------------------
struct Camera
{
	XMVECTOR Pos;
	XMVECTOR Rot;
	Camera() {};
	Camera(XMVECTOR * pos, XMVECTOR * rot) : Pos(*pos), Rot(*rot) {};
	Camera(const XMVECTOR & pos, const XMVECTOR & rot) : Pos(pos), Rot(rot) {};
	XMMATRIX GetViewMatrix()
	{
		XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, -1, 0), Rot);
		return(XMMatrixLookAtRH(Pos, XMVectorAdd(Pos, forward), XMVector3Rotate(XMVectorSet(0, 1, 0, 0), Rot)));
	}

	static void* operator new(std::size_t size)
	{
		UNREFERENCED_PARAMETER(size);
		return _aligned_malloc(sizeof(Camera), __alignof(Camera));
	}

	static void operator delete(void* p)
	{
		_aligned_free(p);
	}
};

//---------------------------------------------------------------------
struct DirectX11 {
	HWND Window;
	bool Running;
	int WinSizeW;
	int WinSizeH;

	HINSTANCE hInstance;

	DirectX11() : Window(nullptr), Running(false), WinSizeW(0), WinSizeH(0), hInstance(nullptr) {
		
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

	OculusTexture(ovrSession session, int sizeW, int sizeH, int sampleCount = 1) : Session(session), TextureChain(nullptr) {
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = sampleCount;
		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
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
				rtvd.ViewDimension = (sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
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
	// Initialize these to nullptr here to handle device lost failures cleanly
	ovrMirrorTexture mirrorTexture = nullptr;
	OculusTexture* pEyeRenderTexture[2] = { nullptr, nullptr };
	DepthBuffer* pEyeDepthBuffer[2] = { nullptr, nullptr };

	ovrSizei windowSize;

	long long frameIndex = 0;
	int msaaRate = 4;

	bool isVisible = true;

	ovrSession session;
	ovrHmdDesc hmdDesc;

	ovrPosef EyeRenderPose[2];
	double sensorSampleTime;

	// Make the eye render buffers (caution if actual size < requested due to HW limits). 
	ovrRecti eyeRenderViewport[2];

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
		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Width = Platform.WinSizeW;
		mirrorDesc.Height = Platform.WinSizeH;
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.MirrorOptions = ovrMirrorOption_Default;
		HRESULT result = ovr_CreateMirrorTextureWithOptionsDX(session, device, &mirrorDesc, &mirrorTexture);
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
	// Initializes LibOVR, and the Rift
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
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


namespace {

	mat4 convert(XMMATRIX& m) {
		XMFLOAT4X4 fView;
		XMStoreFloat4x4(&fView, m);

		mat4 mat;
		mat.Set(0, 0, fView._11); mat.Set(0, 1, fView._12); mat.Set(0, 2, fView._13); mat.Set(0, 3, fView._14);
		mat.Set(1, 0, fView._21); mat.Set(1, 1, fView._22); mat.Set(1, 2, fView._23); mat.Set(1, 3, fView._24);
		mat.Set(2, 0, fView._31); mat.Set(2, 1, fView._32); mat.Set(2, 2, fView._33); mat.Set(2, 3, fView._34);
		mat.Set(3, 0, fView._41); mat.Set(3, 1, fView._42); mat.Set(3, 2, fView._43); mat.Set(3, 3, fView._44);
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

	//Get the pose information in XM format
	XMVECTOR eyeQuat = XMVectorSet(orientation.x, orientation.y, orientation.z, orientation.w);
	XMVECTOR eyePos = XMVectorSet(pos.x, pos.y, pos.z, 0);

	// Get view and projection matrices for the Rift camera
	Camera finalCam(eyePos, eyeQuat);
	XMMATRIX view = finalCam.GetViewMatrix();
	ovrMatrix4f p = ovrMatrix4f_Projection(fov, 0.2f, 1000.0f, ovrProjection_None);
	XMMATRIX proj = XMMatrixSet(p.M[0][0], p.M[1][0], p.M[2][0], p.M[3][0],
								p.M[0][1], p.M[1][1], p.M[2][1], p.M[3][1],
								p.M[0][2], p.M[1][2], p.M[2][2], p.M[3][2],
								p.M[0][3], p.M[1][3], p.M[2][3], p.M[3][3]);

	poseState.vrPose.eye = convert(view).Transpose();
	poseState.vrPose.projection = convert(proj).Transpose();

	ovrSessionStatus sessionStatus;
	ovr_GetSessionStatus(session, &sessionStatus);
	if (sessionStatus.IsVisible) poseState.isVisible = true;
	else poseState.isVisible = false;
	if (sessionStatus.HmdPresent)poseState.hmdPresenting = true;
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

/*VrPoseState VrInterface::getController(int index) {
	return -1;
}*/

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