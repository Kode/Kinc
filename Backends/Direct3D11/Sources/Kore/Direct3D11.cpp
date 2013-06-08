#include "pch.h"
#include <Kore/Math/Core.h>
#include "Direct3D11.h"
#include <Kore/Application.h>
#include "IndexBufferImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Shader.h>
#undef CreateWindow
#include <Kore/System.h>
#include <Kore/WinError.h>
#ifdef SYS_WINDOWSRT
#include <d3d11_1.h>
#include <wrl.h>
#endif

ID3D11Device* device;
ID3D11DeviceContext* context;
ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;

int renderTargetWidth;
int renderTargetHeight;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];

using namespace Kore;

#ifdef SYS_WINDOWSRT
using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
#endif

namespace {
	unsigned hz;
	bool vsync;

	D3D_FEATURE_LEVEL featureLevel;
	ID3D11DepthStencilState* depthTestState = nullptr;
	ID3D11DepthStencilState* noDepthTestState = nullptr;
#ifdef SYS_WINDOWSRT
	IDXGISwapChain1* swapChain;
#else
	IDXGISwapChain* swapChain;
#endif
}

void Graphics::destroy() {

}

void Graphics::init() {
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

	HWND hwnd = (HWND)System::createWindow();

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
#ifdef SYS_WINDOWSRT
		D3D_FEATURE_LEVEL_11_1,
#endif
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	//ID3D11Device* device0;
	//ID3D11DeviceContext* context0;
#ifdef SYS_WINDOWSRT
	affirm(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device, &featureLevel, &context));
#endif
	//affirm(device0.As(&device));
	//affirm(context0.As(&context));

	//m_windowBounds = m_window->Bounds;

	if (swapChain != nullptr) {
		affirm(swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
	}
	else {
#ifdef SYS_WINDOWS
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // 60Hz
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; 
		swapChainDesc.BufferDesc.Width = Application::the()->width();                                     // use automatic sizing
		swapChainDesc.BufferDesc.Height = Application::the()->height();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
		//swapChainDesc.Stereo = false; 
		swapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;                               // use two buffers to enable flip effect
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;//DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;
		swapChainDesc.OutputWindow = (HWND)System::windowHandle();
		swapChainDesc.Windowed = true;
#endif

#ifdef SYS_WINDOWSRT
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = 0;                                     // use automatic sizing
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           // this is the most common swapchain format
		swapChainDesc.Stereo = false; 
		swapChainDesc.SampleDesc.Count = 1;                          // don't use multi-sampling
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;                               // use two buffers to enable flip effect
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;

		IDXGIDevice1* dxgiDevice;
		affirm(device->QueryInterface(IID_IDXGIDevice1, (void**)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter;
		affirm(dxgiDevice->GetAdapter(&dxgiAdapter));

		IDXGIFactory2* dxgiFactory;
		affirm(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));
		
		affirm(dxgiFactory->CreateSwapChainForCoreWindow(device, reinterpret_cast<IUnknown*>(CoreWindow::GetForCurrentThread()), &swapChainDesc, nullptr, &swapChain));
		affirm(dxgiDevice->SetMaximumFrameLatency(1));
#else
		affirm(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 6, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context));
#endif	
	}

	ID3D11Texture2D* backBuffer;
	affirm(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));

	affirm(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));
	
	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);
	renderTargetWidth  = backBufferDesc.Width;
	renderTargetHeight = backBufferDesc.Height;

	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, backBufferDesc.Width, backBufferDesc.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	ID3D11Texture2D* depthStencil;
	affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

	affirm(device->CreateDepthStencilView(depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D), &depthStencilView));

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(backBufferDesc.Width), static_cast<float>(backBufferDesc.Height));
	context->RSSetViewports(1, &viewPort);
	
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = TRUE;
	desc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	device->CreateDepthStencilState(&desc, &depthTestState);
	desc.DepthEnable = FALSE;
	device->CreateDepthStencilState(&desc, &noDepthTestState);
	context->OMSetDepthStencilState(noDepthTestState, 1);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode	= D3D11_FILL_SOLID;
	rasterDesc.CullMode	= D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias	= 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;
	ID3D11RasterizerState* rasterState;
	device->CreateRasterizerState(&rasterDesc, &rasterState);
	context->RSSetState(rasterState);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable			 = true;
	rtbd.SrcBlend				 = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend				 = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp				 = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha			 = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha			 = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha			 = D3D11_BLEND_OP_ADD;
#ifdef SYS_WINDOWSRT
	rtbd.RenderTargetWriteMask	 = D3D11_COLOR_WRITE_ENABLE_ALL;
#else
	rtbd.RenderTargetWriteMask	 = D3D10_COLOR_WRITE_ENABLE_ALL;
#endif

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	ID3D11BlendState* blending;
	device->CreateBlendState(&blendDesc, &blending);

	affirm(device->CreateBlendState(&blendDesc, &blending));
	context->OMSetBlendState(blending, nullptr, 0xffffffff);

#ifdef SYS_WINDOWS
	if (Application::the()->showWindow()) {
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
	}
#endif
}

void Graphics::changeResolution(int width, int height) {

}

void* Graphics::getControl() {
	return nullptr;
}

void Graphics::drawIndexedVertices() {
	Program::setConstants();
	context->DrawIndexed(IndexBuffer::_current->count(), 0, 0);
}

void Graphics::drawIndexedVertices(int start, int count) {
	Program::setConstants();
	context->DrawIndexed(count, start, 0);
}

namespace {
	D3D11_TEXTURE_ADDRESS_MODE convertAddressing(TextureAddressing addressing) {
		switch (addressing) {
		case Repeat:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		default:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}
}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = convertAddressing(addressing);
	samplerDesc.AddressV = convertAddressing(addressing);
	samplerDesc.AddressW = convertAddressing(addressing);
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* sampler;
	affirm(device->CreateSamplerState(&samplerDesc, &sampler));

	context->PSSetSamplers(unit.unit, 1, &sampler);

	sampler->Release();
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {
	if (flags & ClearColorFlag) {
		const float clearColor[] = { ((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f, 1.0f };
		context->ClearRenderTargetView(renderTargetView, clearColor);
	}
	if ((flags & ClearDepthFlag) || (flags & ClearStencilFlag)) {
		uint d3dflags = 
			  (flags & ClearDepthFlag) ? D3D11_CLEAR_DEPTH : 0
			| (flags & ClearStencilFlag) ? D3D11_CLEAR_STENCIL : 0;
		context->ClearDepthStencilView(depthStencilView, d3dflags, depth, stencil);
	}
}

void Graphics::begin() {
#ifdef SYS_WINDOWSRT
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Graphics::end() {
	
}

bool Graphics::isVSynced() {
	return vsync;
}

unsigned Graphics::getHz() {
	return hz;
}

void Graphics::swapBuffers() {
	HRESULT hr = swapChain->Present(1, 0);

	//if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
	//	Initialize(m_window);
	//}
	//else {
		affirm(hr);
	//}
}

namespace {
	//vec4 toVec(const Color& color) {
	//	return vec4(color.R(), color.G(), color.B(), color.A());
	//}
}

void Graphics::setRenderState(RenderState state, bool on) {
	switch (state) {
		case DepthTest:
			if (on) {
				context->OMSetDepthStencilState(depthTestState, 1);
			}
			else {
				context->OMSetDepthStencilState(noDepthTestState, 1);
			}
			break;
		case BackfaceCulling: {
			/*ID3D11RasterizerState* state;
			D3D11_RASTERIZER_DESC desc;

			context->RSGetState(&state);
			//state->GetDesc(&desc);

			desc.CullMode = on ? D3D11_CULL_BACK : D3D11_CULL_NONE;
			device->CreateRasterizerState(&desc, &state);
			context->RSSetState(state);*/
			break;
		}
	}
}

void Graphics::setRenderState(RenderState state, int v) {

}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {

}

void Graphics::setInt(ConstantLocation location, int value) {
	if (location.size == 0) return;
	u8* constants = location.vertex ? vertexConstants : fragmentConstants;
	int* ints = reinterpret_cast<int*>(&constants[location.offset]);
	ints[0] = value;
}

void Graphics::setFloat(ConstantLocation location, float value) {
	if (location.size == 0) return;
	u8* constants = location.vertex ? vertexConstants : fragmentConstants;
	float* floats = reinterpret_cast<float*>(&constants[location.offset]);
	floats[0] = value;
}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {
	if (location.size == 0) return;
	u8* constants = location.vertex ? vertexConstants : fragmentConstants;
	float* floats = reinterpret_cast<float*>(&constants[location.offset]);
	floats[0] = value1;
	floats[1] = value2;
}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	if (location.size == 0) return;
	u8* constants = location.vertex ? vertexConstants : fragmentConstants;
	float* floats = reinterpret_cast<float*>(&constants[location.offset]);
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
	if (location.size == 0) return;
	u8* constants = location.vertex ? vertexConstants : fragmentConstants;
	float* floats = reinterpret_cast<float*>(&constants[location.offset]);
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = value.get(y, x);
		}
	}
}
