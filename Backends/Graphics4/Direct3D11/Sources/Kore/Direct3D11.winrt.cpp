#include "pch.h"

#include <Kore/Log.h>

#include "Direct3D11.h"
#include <Kore/Math/Core.h>
//#include <Kore/Application.h>
#include "IndexBufferImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics4/Shader.h>
#undef CreateWindow
#include <Kore/System.h>
#include <Kore/WinError.h>
#ifdef KORE_WINDOWSAPP
#include <d3d11_1.h>
#include <wrl.h>
#endif
#include <vector>

ID3D11Device* device;
ID3D11DeviceContext* context;
ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* backBuffer;

int renderTargetWidth = 4096;
int renderTargetHeight = 4096;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];
Kore::u8 geometryConstants[1024 * 4];
Kore::u8 tessControlConstants[1024 * 4];
Kore::u8 tessEvalConstants[1024 * 4];

using namespace Kore;

#ifdef KORE_WINDOWSAPP
using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
#endif

namespace Kore {
	extern ProgramImpl* currentProgram;
}

namespace {
	unsigned hz;
	bool vsync;

	D3D_FEATURE_LEVEL featureLevel;
#ifdef KORE_WINDOWSAPP
	IDXGISwapChain1* swapChain;
#else
	IDXGISwapChain* swapChain;
#endif
	int lastStencilReferenceValue = 0;
	D3D11_DEPTH_STENCIL_DESC lastDepthStencil;

	struct DepthStencil {
		D3D11_DEPTH_STENCIL_DESC desc;
		ID3D11DepthStencilState* state;
	};

	std::vector<DepthStencil> depthStencils;

	ID3D11DepthStencilState* getDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc) {
		for (unsigned i = 0; i < depthStencils.size(); ++i) {
			D3D11_DEPTH_STENCIL_DESC& d = depthStencils[i].desc;
			if (desc.DepthEnable == d.DepthEnable && desc.DepthWriteMask == d.DepthWriteMask && desc.DepthFunc == d.DepthFunc &&
			    desc.StencilEnable == d.StencilEnable && desc.StencilReadMask == d.StencilReadMask && desc.StencilWriteMask == d.StencilWriteMask &&
			    desc.FrontFace.StencilFunc == d.FrontFace.StencilFunc && desc.BackFace.StencilFunc == d.BackFace.StencilFunc &&
			    desc.FrontFace.StencilDepthFailOp == d.FrontFace.StencilDepthFailOp && desc.BackFace.StencilDepthFailOp == d.BackFace.StencilDepthFailOp &&
			    desc.FrontFace.StencilPassOp == d.FrontFace.StencilPassOp && desc.BackFace.StencilPassOp == d.BackFace.StencilPassOp &&
			    desc.FrontFace.StencilFailOp == d.FrontFace.StencilFailOp && desc.BackFace.StencilFailOp == d.BackFace.StencilFailOp) {
				return depthStencils[i].state;
			}
		}
		DepthStencil ds;
		ds.desc = desc;
		device->CreateDepthStencilState(&ds.desc, &ds.state);
		depthStencils.push_back(ds);
		return ds.state;
	}

	D3D11_RASTERIZER_DESC lastRasterizer;

	struct Rasterizer {
		D3D11_RASTERIZER_DESC desc;
		ID3D11RasterizerState* state;
	};

	std::vector<Rasterizer> rasterizers;

	ID3D11RasterizerState* getRasterizerState(const D3D11_RASTERIZER_DESC& desc) {
		for (unsigned i = 0; i < rasterizers.size(); ++i) {
			D3D11_RASTERIZER_DESC& d = rasterizers[i].desc;
			if (desc.AntialiasedLineEnable == d.AntialiasedLineEnable && desc.CullMode == d.CullMode && desc.DepthBias == d.DepthBias && desc.DepthBiasClamp == d.DepthBiasClamp
				&& desc.DepthClipEnable == d.DepthClipEnable && desc.FillMode == d.FillMode && desc.FrontCounterClockwise == d.FrontCounterClockwise
				&& desc.MultisampleEnable == d.MultisampleEnable && desc.ScissorEnable == d.ScissorEnable && desc.SlopeScaledDepthBias == d.SlopeScaledDepthBias) {
				return rasterizers[i].state;
			}
		}
		Rasterizer r;
		r.desc = desc;
		device->CreateRasterizerState(&r.desc, &r.state);
		rasterizers.push_back(r);
		return r.state;
	}

	D3D11_SAMPLER_DESC lastSamplers[16];

	struct Sampler {
		D3D11_SAMPLER_DESC desc;
		ID3D11SamplerState* state;
	};

	std::vector<Sampler> samplers;

	ID3D11SamplerState* getSamplerState(const D3D11_SAMPLER_DESC& desc) {
		for (unsigned i = 0; i < samplers.size(); ++i) {
			D3D11_SAMPLER_DESC& d = samplers[i].desc;
			if (desc.AddressU == d.AddressU && desc.AddressV == d.AddressV && desc.AddressW == d.AddressW
				&& desc.BorderColor == d.BorderColor && desc.ComparisonFunc == d.ComparisonFunc && desc.Filter == d.Filter
				&& desc.MaxAnisotropy == d.MaxAnisotropy && desc.MaxLOD == d.MaxLOD && desc.MinLOD == d.MinLOD && desc.MipLODBias == d.MipLODBias) {
				return samplers[i].state;
			}
		}
		Sampler s;
		s.desc = desc;
		device->CreateSamplerState(&s.desc, &s.state);
		samplers.push_back(s);
		return s.state;
	}

	D3D11_COMPARISON_FUNC getComparison(Graphics4::ZCompareMode compare) {
		switch (compare) {
		default:
		case Graphics4::ZCompareAlways:
			return D3D11_COMPARISON_ALWAYS;
		case Graphics4::ZCompareNever:
			return D3D11_COMPARISON_NEVER;
		case Graphics4::ZCompareEqual:
			return D3D11_COMPARISON_EQUAL;
		case Graphics4::ZCompareNotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;
		case Graphics4::ZCompareLess:
			return D3D11_COMPARISON_LESS;
		case Graphics4::ZCompareLessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;
		case Graphics4::ZCompareGreater:
			return D3D11_COMPARISON_GREATER;
		case Graphics4::ZCompareGreaterEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;
		}
	}

	D3D11_STENCIL_OP getStencilAction(Graphics4::StencilAction action) {
		switch (action) {
		default:
		case Graphics4::Keep:
			return D3D11_STENCIL_OP_KEEP;
		case Graphics4::Zero:
			return D3D11_STENCIL_OP_ZERO;
		case Graphics4::Replace:
			return D3D11_STENCIL_OP_REPLACE;
		case Graphics4::Increment:
			return D3D11_STENCIL_OP_INCR;
		case Graphics4::IncrementWrap:
			return D3D11_STENCIL_OP_INCR_SAT;
		case Graphics4::Decrement:
			return D3D11_STENCIL_OP_DECR;
		case Graphics4::DecrementWrap:
			return D3D11_STENCIL_OP_DECR_SAT;
		case Graphics4::Invert:
			return D3D11_STENCIL_OP_INVERT;
		}
	}

	ID3D11RenderTargetView* currentRenderTargetView;
	ID3D11DepthStencilView* currentDepthStencilView;
}

void Graphics4::destroy(int windowId) {}

void Graphics4::init(int windowId, int depthBufferBits, int stencilBufferBits, bool vSync) {
	vsync = vSync;
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

	HWND hwnd = (HWND)System::windowHandle(windowId);

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
#ifdef KORE_WINDOWSAPP
	    D3D_FEATURE_LEVEL_11_1,
#endif
	    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1};

// ID3D11Device* device0;
// ID3D11DeviceContext* context0;
#ifdef KORE_WINDOWSAPP
	affirm(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device,
	                         &featureLevel, &context));
#elif VR_RIFT
	IDXGIFactory* dxgiFactory = nullptr;
	affirm(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&dxgiFactory)));

	affirm(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device, &featureLevel, &context));
#endif
	// affirm(device0.As(&device));
	// affirm(context0.As(&context));

	// m_windowBounds = m_window->Bounds;

	if (swapChain != nullptr) {
		affirm(swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
	}
	else {
#ifdef KORE_WINDOWS
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // 60Hz
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.Width = System::windowWidth(windowId); // use automatic sizing
		swapChainDesc.BufferDesc.Height = System::windowHeight(windowId);
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
		// swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // don't use multi-sampling
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // use two buffers to enable flip effect
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;
		swapChainDesc.OutputWindow = (HWND)System::windowHandle(windowId);
		swapChainDesc.Windowed = true;
#endif

#ifdef KORE_WINDOWSAPP
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = 0; // use automatic sizing
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // don't use multi-sampling
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // use two buffers to enable flip effect
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;

		IDXGIDevice1* dxgiDevice;
		affirm(device->QueryInterface(IID_IDXGIDevice1, (void**)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter;
		affirm(dxgiDevice->GetAdapter(&dxgiAdapter));

		IDXGIFactory2* dxgiFactory;
		affirm(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory));

		affirm(dxgiFactory->CreateSwapChainForCoreWindow(device, reinterpret_cast<IUnknown*>(CoreWindow::GetForCurrentThread()), &swapChainDesc, nullptr,
		                                                 &swapChain));
		affirm(dxgiDevice->SetMaximumFrameLatency(1));

#elif VR_RIFT
		DXGI_SWAP_CHAIN_DESC scDesc = {0};
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Width = System::windowWidth(windowId);
		scDesc.BufferDesc.Height = System::windowHeight(windowId);
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = (HWND)System::windowHandle(windowId);;
		scDesc.SampleDesc.Count = 1;
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

		affirm(dxgiFactory->CreateSwapChain(device, &scDesc, &swapChain));
		dxgiFactory->Release();

		IDXGIDevice1* dxgiDevice = nullptr;
		affirm(device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice));
		affirm(dxgiDevice->SetMaximumFrameLatency(1));
		dxgiDevice->Release();
#else
		UINT flags = 0;

#ifdef _DEBUG
		flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
		affirm(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 6, D3D11_SDK_VERSION, &swapChainDesc, &swapChain,
		                                     &device, nullptr, &context));
#endif
	}

	affirm(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));

	affirm(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));

	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);
	renderTargetWidth = backBufferDesc.Width;
	renderTargetHeight = backBufferDesc.Height;

	// TODO (DK) map depth/stencilBufferBits arguments
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, backBufferDesc.Width, backBufferDesc.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	ID3D11Texture2D* depthStencil;
	affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

	affirm(device->CreateDepthStencilView(depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D), &depthStencilView));

	currentRenderTargetView = renderTargetView;
	currentDepthStencilView = depthStencilView;
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(backBufferDesc.Width), static_cast<float>(backBufferDesc.Height));
	context->RSSetViewports(1, &viewPort);

	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = FALSE;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.StencilEnable = FALSE;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	context->OMSetDepthStencilState(getDepthStencilState(desc), lastStencilReferenceValue);
	lastDepthStencil = desc;

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = 0;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;
	lastRasterizer = rasterDesc;
	context->RSSetState(getRasterizerState(rasterDesc));

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	for (int i = 0; i < 16; ++i) {
		lastSamplers[i] = samplerDesc;
	}

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
#ifdef KORE_WINDOWSAPP
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
#else
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
#endif

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	ID3D11BlendState* blending;
	device->CreateBlendState(&blendDesc, &blending);

	affirm(device->CreateBlendState(&blendDesc, &blending));
	context->OMSetBlendState(blending, nullptr, 0xffffffff);

#ifdef KORE_WINDOWS
	if (System::hasShowWindowFlag()) {
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
	}
#endif

	System::makeCurrent(windowId);
}

void Graphics4::makeCurrent(int contextId) {
	// TODO (DK) implement me
}

void Graphics4::clearCurrent() {
	// TODO (DK) implement me
}

void Graphics4::flush() {}

void Graphics4::changeResolution(int width, int height) {}

void Graphics4::drawIndexedVertices() {
	if (currentProgram->tessControlShader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	Program::setConstants();
	context->DrawIndexed(IndexBuffer::_current->count(), 0, 0);
}

void Graphics4::drawIndexedVertices(int start, int count) {
	if (currentProgram->tessControlShader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	Program::setConstants();
	context->DrawIndexed(count, start, 0);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {
	drawIndexedVerticesInstanced(instanceCount, 0, IndexBuffer::_current->count());
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	if (currentProgram->tessControlShader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	Program::setConstants();
	context->DrawIndexedInstanced(count, instanceCount, start, 0, 0);
}

namespace {
	D3D11_TEXTURE_ADDRESS_MODE convertAddressing(Graphics4::TextureAddressing addressing) {
		switch (addressing) {
		default:
		case Graphics4::Repeat:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case Graphics4::Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case Graphics4::Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case Graphics4::Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		}
	}
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	if (unit.unit < 0) return;
	
	switch (dir) {
	case TexDir::U:
		lastSamplers[unit.unit].AddressU = convertAddressing(addressing);
		break;
	case TexDir::V:
		lastSamplers[unit.unit].AddressV = convertAddressing(addressing);
		break;
	case TexDir::W:
		lastSamplers[unit.unit].AddressW = convertAddressing(addressing);
		break;
	}
	
	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.unit]);
	context->PSSetSamplers(unit.unit, 1, &sampler);
}

void Graphics4::setTexture3DAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Graphics4::setTextureAddressing(unit, dir, addressing);
}

void Graphics4::clear(uint flags, uint color, float depth, int stencil) {
	if (currentRenderTargetView != nullptr && flags & ClearColorFlag) {
		const float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f, 1.0f};
		context->ClearRenderTargetView(currentRenderTargetView, clearColor);
	}
	if (currentDepthStencilView != nullptr && (flags & ClearDepthFlag) || (flags & ClearStencilFlag)) {
		uint d3dflags = ((flags & ClearDepthFlag) ? D3D11_CLEAR_DEPTH : 0) | ((flags & ClearStencilFlag) ? D3D11_CLEAR_STENCIL : 0);
		context->ClearDepthStencilView(currentDepthStencilView, d3dflags, max(0.0f, min(1.0f, depth)), stencil);
	}
}

void Graphics4::begin(int windowId) {
#ifdef KORE_WINDOWSAPP
	// TODO (DK) do i need to do something here?
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif
}

void Graphics4::viewport(int x, int y, int width, int height) {
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	lastRasterizer.ScissorEnable = TRUE;
	context->RSSetState(getRasterizerState(lastRasterizer));
	D3D11_RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	context->RSSetScissorRects(1, &rect);
}

void Graphics4::disableScissor() {
	lastRasterizer.ScissorEnable = FALSE;
	context->RSSetState(getRasterizerState(lastRasterizer));
}

void Graphics4::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue,
                                    int readMask, int writeMask) {
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = lastDepthStencil.DepthEnable;
	desc.DepthWriteMask = lastDepthStencil.DepthWriteMask;
	desc.DepthFunc = lastDepthStencil.DepthFunc;
	desc.StencilEnable = TRUE;
	desc.StencilReadMask = readMask;
	desc.StencilWriteMask = writeMask;
	desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = getComparison(compareMode);
	desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = getStencilAction(depthFail);
	desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = getStencilAction(bothPass);
	desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = getStencilAction(stencilFail);

	lastDepthStencil = desc;
	lastStencilReferenceValue = referenceValue;

	context->OMSetDepthStencilState(getDepthStencilState(desc), lastStencilReferenceValue);
}

void Graphics4::end(int windowId) {}

bool Graphics4::vsynced() {
	return vsync;
}

unsigned Graphics4::refreshRate() {
	return hz;
}

bool Graphics4::swapBuffers(int windowId) {
	HRESULT hr = swapChain->Present(vsync, 0);
	// TODO: if (hr == DXGI_STATUS_OCCLUDED)...
	// http://www.pouet.net/topic.php?which=10454
	// "Proper handling of DXGI_STATUS_OCCLUDED would be to pause the application,
	// and periodically call Present with the TEST flag, and when it returns S_OK, resume rendering."

	// if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
	//	Initialize(m_window);
	//}
	// else {
	return SUCCEEDED(hr);
	//}
}

namespace {
	// vec4 toVec(const Color& color) {
	//	return vec4(color.R(), color.G(), color.B(), color.A());
	//}
}

void Graphics4::setRenderState(RenderState state, bool on) {
	switch (state) {
	case DepthTest: {
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = on;
		desc.DepthWriteMask = lastDepthStencil.DepthWriteMask;
		desc.DepthFunc = lastDepthStencil.DepthFunc;
		desc.StencilEnable = lastDepthStencil.StencilEnable;
		desc.StencilReadMask = lastDepthStencil.StencilReadMask;
		desc.StencilWriteMask = lastDepthStencil.StencilWriteMask;
		desc.FrontFace.StencilFunc = lastDepthStencil.FrontFace.StencilFunc;
		desc.BackFace.StencilFunc = lastDepthStencil.BackFace.StencilFunc;
		desc.FrontFace.StencilDepthFailOp = lastDepthStencil.FrontFace.StencilDepthFailOp;
		desc.BackFace.StencilDepthFailOp = lastDepthStencil.BackFace.StencilDepthFailOp;
		desc.FrontFace.StencilPassOp = lastDepthStencil.FrontFace.StencilPassOp;
		desc.BackFace.StencilPassOp = lastDepthStencil.BackFace.StencilPassOp;
		desc.FrontFace.StencilFailOp = lastDepthStencil.FrontFace.StencilFailOp;
		desc.BackFace.StencilFailOp = lastDepthStencil.BackFace.StencilFailOp;

		lastDepthStencil = desc;

		context->OMSetDepthStencilState(getDepthStencilState(desc), lastStencilReferenceValue);
		break;
	}
	case DepthWrite: {
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = lastDepthStencil.DepthEnable;
		desc.DepthWriteMask = on ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = lastDepthStencil.DepthFunc;
		desc.StencilEnable = lastDepthStencil.StencilEnable;
		desc.StencilReadMask = lastDepthStencil.StencilReadMask;
		desc.StencilWriteMask = lastDepthStencil.StencilWriteMask;
		desc.FrontFace.StencilFunc = lastDepthStencil.FrontFace.StencilFunc;
		desc.BackFace.StencilFunc = lastDepthStencil.BackFace.StencilFunc;
		desc.FrontFace.StencilDepthFailOp = lastDepthStencil.FrontFace.StencilDepthFailOp;
		desc.BackFace.StencilDepthFailOp = lastDepthStencil.BackFace.StencilDepthFailOp;
		desc.FrontFace.StencilPassOp = lastDepthStencil.FrontFace.StencilPassOp;
		desc.BackFace.StencilPassOp = lastDepthStencil.BackFace.StencilPassOp;
		desc.FrontFace.StencilFailOp = lastDepthStencil.FrontFace.StencilFailOp;
		desc.BackFace.StencilFailOp = lastDepthStencil.BackFace.StencilFailOp;

		lastDepthStencil = desc;

		context->OMSetDepthStencilState(getDepthStencilState(desc), lastStencilReferenceValue);
		break;
	}
	case BackfaceCulling: {
		lastRasterizer.CullMode = on ? D3D11_CULL_BACK : D3D11_CULL_NONE;
		context->RSSetState(getRasterizerState(lastRasterizer));
		break;
	}
	}
}

void Graphics4::setRenderState(RenderState state, int v) {
	switch (state) {
	case DepthTestCompare:
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = lastDepthStencil.DepthEnable;
		desc.DepthWriteMask = lastDepthStencil.DepthWriteMask;
		desc.DepthFunc = getComparison((ZCompareMode)v);
		desc.StencilEnable = lastDepthStencil.StencilEnable;
		desc.StencilReadMask = lastDepthStencil.StencilReadMask;
		desc.StencilWriteMask = lastDepthStencil.StencilWriteMask;
		desc.FrontFace.StencilFunc = lastDepthStencil.FrontFace.StencilFunc;
		desc.BackFace.StencilFunc = lastDepthStencil.BackFace.StencilFunc;
		desc.FrontFace.StencilDepthFailOp = lastDepthStencil.FrontFace.StencilDepthFailOp;
		desc.BackFace.StencilDepthFailOp = lastDepthStencil.BackFace.StencilDepthFailOp;
		desc.FrontFace.StencilPassOp = lastDepthStencil.FrontFace.StencilPassOp;
		desc.BackFace.StencilPassOp = lastDepthStencil.BackFace.StencilPassOp;
		desc.FrontFace.StencilFailOp = lastDepthStencil.FrontFace.StencilFailOp;
		desc.BackFace.StencilFailOp = lastDepthStencil.BackFace.StencilFailOp;

		lastDepthStencil = desc;

		context->OMSetDepthStencilState(getDepthStencilState(desc), lastStencilReferenceValue);
		break;
	}
}

void Graphics4::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	// TODO
}

namespace {
	void setInt(u8* constants, u8 offset, u8 size, int value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(u8* constants, u8 offset, u8 size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, u8 offset, u8 size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, u8 offset, u8 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, u8 offset, u8 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(u8* constants, u8 offset, u8 size, float* values, int count) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}

	void setBool(u8* constants, u8 offset, u8 size, bool value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8* constants, u8 offset, u8 size, const mat4& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}

	void setMatrix(u8* constants, u8 offset, u8 size, const mat3& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}
}

void Graphics4::setInt(ConstantLocation location, int value) {
	::setInt(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setInt(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setInt(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setInt(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setInt(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	::setFloat(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setFloat(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setFloat(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setFloat(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setFloat(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	::setFloat2(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2);
	::setFloat2(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2);
	::setFloat2(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2);
	::setFloat2(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2);
	::setFloat2(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2);
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	::setFloat3(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3);
	::setFloat3(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3);
	::setFloat3(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3);
	::setFloat3(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3);
	::setFloat3(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3);
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	::setFloat4(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3, value4);
	::setFloat4(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3, value4);
	::setFloat4(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3, value4);
	::setFloat4(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3, value4);
	::setFloat4(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3, value4);
}

void Graphics4::setFloats(ConstantLocation location, float* values, int count) {
	::setFloats(vertexConstants, location.vertexOffset, location.vertexSize, values, count);
	::setFloats(fragmentConstants, location.fragmentOffset, location.fragmentSize, values, count);
	::setFloats(geometryConstants, location.geometryOffset, location.geometrySize, values, count);
	::setFloats(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, values, count);
	::setFloats(tessControlConstants, location.tessControlOffset, location.tessControlSize, values, count);
}

void Graphics4::setBool(ConstantLocation location, bool value) {
	::setBool(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setBool(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setBool(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setBool(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setBool(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

/*
case D3D11_FILTER_MIN_MAG_MIP_POINT:
	break;
case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
	break;
case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
	break;
case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
	break;
case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
	break;
case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
	break;
case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
	break;
case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
	break;
case D3D11_FILTER_ANISOTROPIC:
	break;
*/

void Graphics4::setTextureMagnificationFilter(TextureUnit unit, TextureFilter filter) {
	if (unit.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case PointFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		}
		break;
	case LinearFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		break;
	case AnisotropicFilter:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.unit].Filter = d3d11filter;
	
	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.unit]);
	context->PSSetSamplers(unit.unit, 1, &sampler);
}

void Graphics4::setTexture3DMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMagnificationFilter(texunit, filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit unit, TextureFilter filter) {
	if (unit.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case PointFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		}
		break;
	case LinearFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		break;
	case AnisotropicFilter:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.unit].Filter = d3d11filter;

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.unit]);
	context->PSSetSamplers(unit.unit, 1, &sampler);
}

void Graphics4::setTexture3DMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMinificationFilter(texunit, filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit unit, MipmapFilter filter) {
	if (unit.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case PointFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		}
		break;
	case LinearFilter:
		switch (lastSamplers[unit.unit].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		}
		break;
	case AnisotropicFilter:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.unit].Filter = d3d11filter;

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.unit]);
	context->PSSetSamplers(unit.unit, 1, &sampler);
}

void Graphics4::setTexture3DMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Graphics4::setTextureMipmapFilter(texunit, filter);
}

namespace {
	D3D11_BLEND convert(Graphics4::BlendingOperation operation) {
		switch (operation) {
		case Graphics4::BlendOne:
			return D3D11_BLEND_ONE;
		case Graphics4::BlendZero:
			return D3D11_BLEND_ZERO;
		case Graphics4::SourceAlpha:
			return D3D11_BLEND_SRC_ALPHA;
		case Graphics4::DestinationAlpha:
			return D3D11_BLEND_DEST_ALPHA;
		case Graphics4::InverseSourceAlpha:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case Graphics4::InverseDestinationAlpha:
			return D3D11_BLEND_INV_DEST_ALPHA;
		default:
			//	throw Exception("Unknown blending operation.");
			return D3D11_BLEND_SRC_ALPHA;
		}
	}

	Graphics4::BlendingOperation lastSource = Graphics4::SourceAlpha;
	Graphics4::BlendingOperation lastDestination = Graphics4::InverseSourceAlpha;
	bool lastRed = true;
	bool lastGreen = true;
	bool lastBlue = true;
	bool lastAlpha = true;
	ID3D11BlendState* blendState = nullptr;

	void setBlendState(Graphics4::BlendingOperation source, Graphics4::BlendingOperation destination, bool red, bool green, bool blue, bool alpha) {
		lastSource = source;
		lastDestination = destination;
		lastRed = red;
		lastGreen = green;
		lastBlue = blue;
		lastAlpha = alpha;

		if (blendState != nullptr) {
			blendState->Release();
		}

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));

		D3D11_RENDER_TARGET_BLEND_DESC rtbd;
		ZeroMemory(&rtbd, sizeof(rtbd));

		rtbd.BlendEnable = source != Graphics4::BlendOne || destination != Graphics4::BlendZero;
		rtbd.SrcBlend = convert(source);
		rtbd.DestBlend = convert(destination);
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = convert(source);
		rtbd.DestBlendAlpha = convert(destination);
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask =
		    ((((red ? D3D11_COLOR_WRITE_ENABLE_RED : 0) | (green ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0)) | (blue ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0)) |
		     (alpha ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0));

		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.RenderTarget[0] = rtbd;

		device->CreateBlendState(&blendDesc, &blendState);

		float blendFactor[] = {0, 0, 0, 0};
		UINT sampleMask = 0xffffffff;
		context->OMSetBlendState(blendState, blendFactor, sampleMask);
	}
}

void Graphics4::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	setBlendState(source, destination, lastRed, lastGreen, lastBlue, lastAlpha);
}

void Graphics4::setBlendingModeSeparate(BlendingOperation source, BlendingOperation destination, BlendingOperation alphaSource, BlendingOperation alphaDestination) {
	// TODO
}

void Graphics4::setColorMask(bool red, bool green, bool blue, bool alpha) {
	setBlendState(lastSource, lastDestination, red, green, blue, alpha);
}

bool Graphics4::renderTargetsInvertedY() {
	return false;
}

bool Graphics4::nonPow2TexturesSupported() {
	return true;
}

void Graphics4::restoreRenderTarget() {
	currentRenderTargetView = renderTargetView;
	currentDepthStencilView = depthStencilView;
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(renderTargetWidth), static_cast<float>(renderTargetHeight));
	context->RSSetViewports(1, &viewPort);
}

void Graphics4::setRenderTargets(RenderTarget** targets, int count) {
	for (int i = 0; i < count; ++i) {
		if (targets[i]->lastBoundUnit >= 0) {
			ID3D11ShaderResourceView* nullview[1];
			nullview[0] = nullptr;
			context->PSSetShaderResources(targets[i]->lastBoundUnit, 1, nullview);
		}
	}
	
	currentRenderTargetView = targets[0]->renderTargetView;
	currentDepthStencilView = targets[0]->depthStencilView;
	
	ID3D11RenderTargetView** renderViews = (ID3D11RenderTargetView**)alloca(sizeof(ID3D11RenderTargetView*) * count);
	for (int i = 0; i < count; ++i) {
		renderViews[i] = targets[i]->renderTargetView;
	}

	context->OMSetRenderTargets(count, renderViews, targets[0]->depthStencilView);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(targets[0]->width), static_cast<float>(targets[0]->height));
	context->RSSetViewports(1, &viewPort);
}

void Graphics4::setRenderTargetFace(RenderTarget* texture, int face) {
	// TODO
}

void Graphics4::setVertexBuffers(VertexBuffer** buffers, int count) {
	buffers[0]->_set(0);

	ID3D11Buffer** d3dbuffers = (ID3D11Buffer**)alloca(count * sizeof(ID3D11Buffer*));
	for (int i = 0; i < count; ++i) {
		d3dbuffers[i] = buffers[i]->_vb;
	}

	UINT* strides = (UINT*)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		strides[i] = buffers[i]->myStride;
	}

	UINT* internaloffsets = (UINT*)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		internaloffsets[i] = 0;
	}

	context->IASetVertexBuffers(0, count, d3dbuffers, strides, internaloffsets);
}

void Graphics4::setIndexBuffer(IndexBuffer& buffer) {
	buffer._set();
}

void Graphics4::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

void Graphics4::setImageTexture(TextureUnit unit, Texture* texture) {
	// TODO
}

void Graphics4::setup() {}

uint queryCount = 0;
std::vector<ID3D11Query*> queryPool;

bool Graphics4::initOcclusionQuery(uint* occlusionQuery) {
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_OCCLUSION;
	queryDesc.MiscFlags = 0;
	ID3D11Query* pQuery = nullptr;
	HRESULT result = device->CreateQuery(&queryDesc, &pQuery);

	if (FAILED(result)) {
		Kore::log(Kore::LogLevel::Info, "Internal query creation failed, result: 0x%X.", result);
		return false;
	}

	queryPool.push_back(pQuery);
	*occlusionQuery = queryCount;
	++queryCount;

	return true;
}

void Graphics4::deleteOcclusionQuery(uint occlusionQuery) {
	if (occlusionQuery < queryPool.size()) queryPool[occlusionQuery] = nullptr;
}

void Graphics4::renderOcclusionQuery(uint occlusionQuery, int triangles) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Program::setConstants();
		context->Begin(pQuery);
		context->Draw(triangles, 0);
		context->End(pQuery);
	}
}

bool Graphics4::isQueryResultsAvailable(uint occlusionQuery) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		if (S_OK == context->GetData(pQuery, 0, 0, 0)) return true;
	}
	return false;
}

void Graphics4::getQueryResults(uint occlusionQuery, uint* pixelCount) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		UINT64 numberOfPixelsDrawn;
		HRESULT result = context->GetData(pQuery, &numberOfPixelsDrawn, sizeof(UINT64), 0);
		if (S_OK == result) {
			*pixelCount = static_cast<uint>(numberOfPixelsDrawn);
		}
		else {
			Kore::log(Kore::LogLevel::Info, "Check first if results are available");
			*pixelCount = 0;
		}
	}
}
