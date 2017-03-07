#include "pch.h"

#include <Kore/Vr/VrInterface.h>

#ifdef VR_RIFT

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

#include "OVR_CAPI_D3D.h"

#include <cstdint>
#include <vector>
#include "d3dcompiler.h"
#include "d3d11.h"
#include "stdio.h"
#include <new>
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

// clean up member COM pointers
template<typename T> void Release(T *&obj)
{
	if (!obj) return;
	obj->Release();
	obj = nullptr;
}

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
		Release(TexDsv);
	}
};

//----------------------------------------------------------------
struct DataBuffer {
	ID3D11Buffer* D3DBuffer;
	size_t Size;

	DataBuffer(ID3D11Device * Device, D3D11_BIND_FLAG use, const void* buffer, size_t size) : Size(size) {
		D3D11_BUFFER_DESC desc;   memset(&desc, 0, sizeof(desc));
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.BindFlags = use;
		desc.ByteWidth = (unsigned)size;
		D3D11_SUBRESOURCE_DATA sr;
		sr.pSysMem = buffer;
		sr.SysMemPitch = sr.SysMemSlicePitch = 0;
		Device->CreateBuffer(&desc, buffer ? &sr : NULL, &D3DBuffer);
	}
	~DataBuffer() {
		Release(D3DBuffer);
	}
};

//---------------------------------------------------------------------
struct DirectX11 {
	HWND Window;
	bool Running;
	bool Key[256];
	int WinSizeW;
	int WinSizeH;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	IDXGISwapChain* SwapChain;
	DepthBuffer* MainDepthBuffer;
	ID3D11Texture2D* BackBuffer;
	ID3D11RenderTargetView* BackBufferRT;
	// Fixed size buffer for shader constants, before copied into buffer
	static const int UNIFORM_DATA_SIZE = 2000;
	unsigned char UniformData[UNIFORM_DATA_SIZE];
	DataBuffer* UniformBufferGen;		// TODO: Delete?
	HINSTANCE hInstance;

	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
		auto p = reinterpret_cast<DirectX11 *>(GetWindowLongPtr(hWnd, 0));
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

	DirectX11() : Window(nullptr), Running(false), WinSizeW(0), WinSizeH(0), Device(nullptr), Context(nullptr), SwapChain(nullptr),
		MainDepthBuffer(nullptr), BackBuffer(nullptr), BackBufferRT(nullptr), UniformBufferGen(nullptr), hInstance(nullptr) {
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~DirectX11() {
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hinst, LPCWSTR title) {
		hInstance = hinst;
		Running = true;

		WNDCLASSW wc;
		memset(&wc, 0, sizeof(wc));
		wc.lpszClassName = L"App";
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbWndExtra = sizeof(this);
		RegisterClassW(&wc);

		// adjust the window size and show at InitDevice time
		Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hinst, 0);
		if (!Window) return false;

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		return true;
	}

	void CloseWindow() {
		if (Window) {
			DestroyWindow(Window);
			Window = nullptr;
			UnregisterClassW(L"App", hInstance);
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

		IDXGIFactory * DXGIFactory = nullptr;
		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
		if (hr != ERROR_SUCCESS) {
			log(Warning, "CreateDXGIFactory1 failed");
			return false;
		}

		IDXGIAdapter * Adapter = nullptr;
		for (UINT iAdapter = 0; DXGIFactory->EnumAdapters(iAdapter, &Adapter) != DXGI_ERROR_NOT_FOUND; ++iAdapter) {
			DXGI_ADAPTER_DESC adapterDesc;
			Adapter->GetDesc(&adapterDesc);
			if ((pLuid == nullptr) || memcmp(&adapterDesc.AdapterLuid, pLuid, sizeof(LUID)) == 0)
				break;
			Release(Adapter);
		}

		auto DriverType = Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
		hr = D3D11CreateDevice(Adapter, DriverType, 0, 0, 0, 0, D3D11_SDK_VERSION, &Device, 0, &Context);
		Release(Adapter);
		if (hr != ERROR_SUCCESS) {
			log(Warning, "D3D11CreateDevice failed");
			return false;
		}

		// Create swap chain
		DXGI_SWAP_CHAIN_DESC scDesc;
		memset(&scDesc, 0, sizeof(scDesc));
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Width = WinSizeW;
		scDesc.BufferDesc.Height = WinSizeH;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = Window;
		scDesc.SampleDesc.Count = 1;
		scDesc.Windowed = windowed;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
		hr = DXGIFactory->CreateSwapChain(Device, &scDesc, &SwapChain);
		Release(DXGIFactory);
		if (hr != ERROR_SUCCESS) {
			log(Warning, "CreateSwapChain failed");
			return false;
		}

		// Create backbuffer
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
		hr = Device->CreateRenderTargetView(BackBuffer, NULL, &BackBufferRT);
		if (hr != ERROR_SUCCESS) {
			log(Warning, "CreateRenderTargetView failed");
			return false;
		}

		// Main depth buffer
		MainDepthBuffer = new DepthBuffer(Device, WinSizeW, WinSizeH);
		Context->OMSetRenderTargets(1, &BackBufferRT, MainDepthBuffer->TexDsv);

		// Buffer for shader constants
		UniformBufferGen = new DataBuffer(Device, D3D11_BIND_CONSTANT_BUFFER, NULL, UNIFORM_DATA_SIZE);
		Context->VSSetConstantBuffers(0, 1, &UniformBufferGen->D3DBuffer);

		// Set max frame latency to 1
		IDXGIDevice1* DXGIDevice1 = nullptr;
		hr = Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice1);
		if (hr != ERROR_SUCCESS) {
			log(Warning, "QueryInterface failed");
			return false;
		}
		DXGIDevice1->SetMaximumFrameLatency(1);
		Release(DXGIDevice1);

		return true;
	}

	void SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, struct DepthBuffer * depthbuffer, float R = 0, float G = 0, float B = 0, float A = 0) {
		float black[] = { R, G, B, A }; // Important that alpha=0, if want pixels to be transparent, for manual layers
		Context->OMSetRenderTargets(1, &rendertarget, (depthbuffer ? depthbuffer->TexDsv : nullptr));
		Context->ClearRenderTargetView(rendertarget, black);
		if (depthbuffer)
			Context->ClearDepthStencilView(depthbuffer->TexDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	}

	void SetViewport(float vpX, float vpY, float vpW, float vpH) {
		D3D11_VIEWPORT D3Dvp;
		D3Dvp.Width = vpW;    D3Dvp.Height = vpH;
		D3Dvp.MinDepth = 0;   D3Dvp.MaxDepth = 1;
		D3Dvp.TopLeftX = vpX; D3Dvp.TopLeftY = vpY;
		Context->RSSetViewports(1, &D3Dvp);
	}

	void ReleaseDevice() {
		Release(BackBuffer);
		Release(BackBufferRT);
		if (SwapChain) {
			SwapChain->SetFullscreenState(FALSE, NULL);
			Release(SwapChain);
		}
		Release(Context);
		Release(Device);
		delete MainDepthBuffer;
		MainDepthBuffer = nullptr;
		delete UniformBufferGen;
		UniformBufferGen = nullptr;
	}
};

static DirectX11 Platform;

//---------------------------------------------------------------------

// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.
struct OculusTexture {
	ovrSession Session;
	ovrTextureSwapChain TextureChain;
	std::vector<ID3D11RenderTargetView*> TexRtv;

	OculusTexture() : Session(nullptr), TextureChain(nullptr) {}

	bool Init(ovrSession session, int sizeW, int sizeH) {
		Session = session;

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

		ovrResult result = ovr_CreateTextureSwapChainDX(session, Platform.Device, &desc, &TextureChain);
		if (!OVR_SUCCESS(result))
			return false;

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(Session, TextureChain, &textureCount);
		for (int i = 0; i < textureCount; ++i) {
			ID3D11Texture2D* tex = nullptr;
			ovr_GetTextureSwapChainBufferDX(Session, TextureChain, i, IID_PPV_ARGS(&tex));
			D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
			rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			ID3D11RenderTargetView* rtv;
			Platform.Device->CreateRenderTargetView(tex, &rtvd, &rtv);
			TexRtv.push_back(rtv);
			tex->Release();
		}

		return true;
	}

	~OculusTexture() {
		for (int i = 0; i < (int)TexRtv.size(); ++i) {
			Release(TexRtv[i]);
		}
		if (TextureChain) {
			ovr_DestroyTextureSwapChain(Session, TextureChain);
		}
	}

	ID3D11RenderTargetView* GetRTV() {
		int index = 0;
		ovr_GetTextureSwapChainCurrentIndex(Session, TextureChain, &index);
		return TexRtv[index];
	}

	// Commit changes
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
}

void* VrInterface::init(void* hinst) {
	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	if (!OVR_SUCCESS(result)) {
		log(Warning, "Failed to initialize libOVR.");
		return(0);
	}

	if (!Platform.InitWindow((HINSTANCE)hinst, L"Oculus Rift")) {
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
	windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
	if (!Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&luid))) {
		log(Info, "Failed to init device.");
		done();
	}

	// Make eye render buffers
	for (int eye = 0; eye < 2; ++eye) {
		ovrSizei idealSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
		pEyeRenderTexture[eye] = new OculusTexture();
		if (!pEyeRenderTexture[eye]->Init(session, idealSize.w, idealSize.h)) {
			log(Warning, "Failed to create eye texture.");
			done();
		}
		pEyeDepthBuffer[eye] = new DepthBuffer(Platform.Device, idealSize.w, idealSize.h);
		eyeRenderViewport[eye].Pos.x = 0;
		eyeRenderViewport[eye].Pos.y = 0;
		eyeRenderViewport[eye].Size = idealSize;
		if (!pEyeRenderTexture[eye]->TextureChain) {
			log(Warning, "Failed to create texture.");
			done();
		}
	}

	ovrMirrorTextureDesc desc;
	memset(&desc, 0, sizeof(desc));
	desc.Width = Platform.WinSizeW;
	desc.Height = Platform.WinSizeH;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	result = ovr_CreateMirrorTextureDX(session, Platform.Device, &desc, &mirrorTexture);
	if (!OVR_SUCCESS(result)) {
		log(Info, "Failed to create mirror texture.");
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

			//log(Info, "viewport %i %i %i %i", ld.Viewport[eye].Size.w, ld.Viewport[eye].Size.h, ld.Viewport[eye].Pos.x, ld.Viewport[eye].Pos.y);
			//log(Info, "Fov %f %f %f %f", ld.Fov[eye].UpTan, ld.Fov[eye].DownTan, ld.Fov[eye].LeftTan, ld.Fov[eye].RightTan);
			//log(Info, "sensorSampleTime %i", sensorSampleTime);
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
	Platform.Context->CopyResource(Platform.BackBuffer, tex);
	tex->Release();
	Platform.SwapChain->Present(0, 0);
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