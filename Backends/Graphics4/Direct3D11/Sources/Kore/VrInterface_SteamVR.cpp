#include "pch.h"

#ifdef KORE_STEAMVR

#include <Kore/Vr/VrInterface.h>

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>
#include "Direct3D11.h"

#include <vector>
#include "d3d11.h"
#if _MSC_VER > 1600
#include "DirectXMath.h"
using namespace DirectX;
#else
#include "xnamath.h"
#endif //_MSC_VER > 1600

#include <openvr.h>

using namespace Kore;

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
		ID3D11Texture2D* Tex;
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

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName) {
	vr::HmdError error;
	vr::VR_Init(&error, vr::VRApplication_Scene);

	// Return window
	return Platform.Window;
}

void VrInterface::begin() {

}

void VrInterface::beginRender(int eye) {

}

void VrInterface::endRender(int eye) {

}

SensorState* VrInterface::getSensorState(int eye) {
	SensorState* sensorState = new SensorState();
	VrPoseState* poseState = new VrPoseState();

	VrPoseState* predictedPoseState = new VrPoseState();

	return sensorState;
}

void VrInterface::warpSwap() {

}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin) {

}

void VrInterface::resetHmdPose() {

}

void VrInterface::ovrShutdown() {

}

#endif