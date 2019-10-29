#include "pch.h"

#define NOMINMAX

#include <kinc/log.h>

#include "Direct3D11.h"
#include <kinc/math/core.h>
//#include <Kore/Application.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texturearray.h>

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/vertexbuffer.h>

#undef CreateWindow

#include <Kore/System.h>
#include <Kore/SystemMicrosoft.h>

#include <kinc/display.h>
#include <kinc/window.h>

#ifdef KORE_WINDOWS
#include <Kore/Windows.h>
#else
int antialiasingSamples() {
	return 1;
}
#endif

#ifdef KORE_WINDOWSAPP
#include <d3d11_1.h>
#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <wrl.h>
#endif

#include <vector>

#include <stdint.h>
#include <malloc.h>

#ifdef KORE_HOLOLENS
#include "DeviceResources.winrt.h"
#include "Hololens.winrt.h"
#include <memory>
#include <windows.graphics.directx.direct3d11.interop.h>
#endif

ID3D11Device *device;
ID3D11DeviceContext *context;
ID3D11RenderTargetView *renderTargetView;
ID3D11Texture2D *depthStencil;
ID3D11DepthStencilView *depthStencilView;
ID3D11Texture2D *backBuffer;

int renderTargetWidth = 4096;
int renderTargetHeight = 4096;
int newRenderTargetWidth = 4096;
int newRenderTargetHeight = 4096;

uint8_t vertexConstants[1024 * 4];
uint8_t fragmentConstants[1024 * 4];
uint8_t geometryConstants[1024 * 4];
uint8_t tessControlConstants[1024 * 4];
uint8_t tessEvalConstants[1024 * 4];

#ifdef KORE_WINDOWSAPP
using namespace ::Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
#ifdef KORE_HOLOLENS
using namespace Windows::Graphics::Holographic;
using namespace Windows::Graphics::DirectX::Direct3D11;
#endif
#endif

extern kinc_g4_pipeline_t *currentPipeline;
void kinc_internal_set_constants(void);

bool kinc_internal_scissoring = false;

namespace {
	unsigned hz;
	bool vsync;

	D3D_FEATURE_LEVEL featureLevel;
#ifdef KORE_WINDOWSAPP
	IDXGISwapChain1* swapChain = nullptr;
#else
	IDXGISwapChain* swapChain = nullptr;
#endif
	D3D11_SAMPLER_DESC lastSamplers[16];

	struct Sampler {
		D3D11_SAMPLER_DESC desc;
		ID3D11SamplerState* state;
	};

	std::vector<Sampler> samplers;

	ID3D11SamplerState* getSamplerState(const D3D11_SAMPLER_DESC& desc) {
		for (unsigned i = 0; i < samplers.size(); ++i) {
			if (memcmp(&desc, &samplers[i].desc, sizeof(D3D11_SAMPLER_DESC)) == 0) {
				return samplers[i].state;
			}
		}
		Sampler s;
		s.desc = desc;
		device->CreateSamplerState(&s.desc, &s.state);
		samplers.push_back(s);
		return s.state;
	}

	void initSamplers() {
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ID3D11SamplerState* state;
		device->CreateSamplerState(&samplerDesc, &state);

		ID3D11SamplerState* states[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
		for (int i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i) {
			states[i] = state;
		}

		context->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, states);
		context->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, states);
	}

	ID3D11RenderTargetView** currentRenderTargetViews =
	    (ID3D11RenderTargetView**)malloc(sizeof(ID3D11RenderTargetView*) * D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
	int renderTargetCount = 1;
	ID3D11DepthStencilView* currentDepthStencilView;
} // namespace

void kinc_g4_destroy(int window) {}

static void createBackbuffer(int antialiasingSamples) {
	kinc_microsoft_affirm(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer));

	kinc_microsoft_affirm(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));

	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);
	newRenderTargetWidth = renderTargetWidth = backBufferDesc.Width;
	newRenderTargetHeight = renderTargetHeight = backBufferDesc.Height;

	// TODO (DK) map depth/stencilBufferBits arguments
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, backBufferDesc.Width, backBufferDesc.Height, 1, 1, D3D11_BIND_DEPTH_STENCIL,
	                                       D3D11_USAGE_DEFAULT, 0U, antialiasingSamples > 1 ? antialiasingSamples : 1,
	                                       antialiasingSamples > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0);

	kinc_microsoft_affirm(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

	kinc_microsoft_affirm(device->CreateDepthStencilView(
	    depthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(antialiasingSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D),
	    &depthStencilView));
}

#ifdef KORE_WINDOWS
static bool isWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) {
	OSVERSIONINFOEXW osvi = {sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0};
	DWORDLONG const dwlConditionMask =
	    VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL),
	                        VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8 0x0602
#endif

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif

static bool isWindows8OrGreater() {
	return isWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
}

static bool isWindows10OrGreater() {
	return isWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);
}
#endif

void kinc_g4_init(int windowId, int depthBufferBits, int stencilBufferBits, bool vSync) {
#ifdef KORE_VR
	vsync = false;
#else
	vsync = vSync;
#endif
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

#ifdef KORE_WINDOWS
	HWND hwnd = kinc_windows_window_handle(windowId);
#endif

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
#ifdef KORE_WINDOWSAPP
	    D3D_FEATURE_LEVEL_11_1,
#endif
	    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

#ifdef KORE_WINDOWSAPP
	IDXGIAdapter3* adapter = nullptr;
#ifdef KORE_HOLOLENS
	adapter = holographicFrameController->getCompatibleDxgiAdapter().Get();
#endif
	kinc_microsoft_affirm(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels),
	                                        D3D11_SDK_VERSION,
	                                    &device, &featureLevel, &context));

#elif KORE_OCULUS
	IDXGIFactory* dxgiFactory = nullptr;
	Windows::affirm(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&dxgiFactory)));

	Windows::affirm(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device,
	                                  &featureLevel, &context));
#endif
	// affirm(device0.As(&device));
	// affirm(context0.As(&context));

	// m_windowBounds = m_window->Bounds;

	const int _DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL = 3;
	const int _DXGI_SWAP_EFFECT_FLIP_DISCARD = 4;

	if (swapChain != nullptr) {
		kinc_microsoft_affirm(swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
	}
	else {
#ifdef KORE_WINDOWS
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // 60Hz
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.Width = kinc_window_width(windowId); // use automatic sizing
		swapChainDesc.BufferDesc.Height = kinc_window_height(windowId);
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
		// swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = kinc_g4_antialiasing_samples() > 1 ? kinc_g4_antialiasing_samples() : 1;
		swapChainDesc.SampleDesc.Quality = kinc_g4_antialiasing_samples() > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // use two buffers to enable flip effect
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // DXGI_SCALING_NONE;
		if (isWindows10OrGreater()) {
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			//(DXGI_SWAP_EFFECT) _DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}
		else if (isWindows8OrGreater()) {
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			//(DXGI_SWAP_EFFECT) _DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		}
		else {
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		}
		swapChainDesc.Flags = 0;
		swapChainDesc.OutputWindow = kinc_windows_window_handle(windowId);
		swapChainDesc.Windowed = true;
#endif

#if defined(KORE_WINDOWSAPP)
#ifdef KORE_HOLOLENS
		// The Windows::Graphics::Holographic::HolographicSpace owns its own swapchain so we don't need to create one here
#else
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = 0; // use automatic sizing
		swapChainDesc.Height = 0;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = antialiasingSamples() > 1 ? antialiasingSamples() : 1;
		swapChainDesc.SampleDesc.Quality = antialiasingSamples() > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // use two buffers to enable flip effect
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications
		swapChainDesc.Flags = 0;

		IDXGIDevice1* dxgiDevice;
		kinc_microsoft_affirm(device->QueryInterface(IID_IDXGIDevice1, (void **)&dxgiDevice));

		IDXGIAdapter* dxgiAdapter;
		kinc_microsoft_affirm(dxgiDevice->GetAdapter(&dxgiAdapter));

		IDXGIFactory2* dxgiFactory;
		kinc_microsoft_affirm(dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&dxgiFactory));

		kinc_microsoft_affirm(dxgiFactory->CreateSwapChainForCoreWindow(device, reinterpret_cast<IUnknown *>(CoreWindow::GetForCurrentThread()), &swapChainDesc,
		                                                            nullptr, &swapChain));
		kinc_microsoft_affirm(dxgiDevice->SetMaximumFrameLatency(1));
#endif

#elif KORE_OCULUS
		DXGI_SWAP_CHAIN_DESC scDesc = {0};
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Width = System::windowWidth(windowId);
		scDesc.BufferDesc.Height = System::windowHeight(windowId);
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.OutputWindow = (HWND)System::windowHandle(windowId);
		;
		scDesc.SampleDesc.Count = antialiasingSamples() > 1 ? antialiasingSamples() : 1;
		scDesc.SampleDesc.Quality = antialiasingSamples() > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

		Windows::affirm(dxgiFactory->CreateSwapChain(device, &scDesc, &swapChain));
		dxgiFactory->Release();

		IDXGIDevice1* dxgiDevice = nullptr;
		Windows::affirm(device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice));
		Windows::affirm(dxgiDevice->SetMaximumFrameLatency(1));
		dxgiDevice->Release();
#else
		UINT flags = 0;

#ifdef _DEBUG
		flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 3, D3D11_SDK_VERSION, &swapChainDesc,
		                                               &swapChain, &device, nullptr, &context);
		if (result != S_OK) {
			kinc_microsoft_affirm(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, featureLevels, 3, D3D11_SDK_VERSION,
			                                                    &swapChainDesc,
			                                                &swapChain, &device, nullptr, &context));
		}
#endif
	}

#ifdef KORE_HOLOLENS
	// holographicFrameController manages the targets and views for hololens.
	// the views have to be created/deleted on the CameraAdded/Removed events
	// at this point we don't know if this event has alread occured so we cannot
	// simply set the renderTargetWidth, renderTargetHeight, currentRenderTargetViews and currentDepthStencilView.
	// to bind the targets for hololens one has to use the VrInterface::beginRender(eye) instead of the methods in this class.
	ComPtr<ID3D11Device> devicePtr = device;
	ComPtr<ID3D11DeviceContext> contextPtr = context;
	Microsoft::WRL::ComPtr<ID3D11Device4> device4Ptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext3> context3Ptr;
	affirm(devicePtr.As(&device4Ptr));
	affirm(contextPtr.As(&context3Ptr));
	holographicFrameController->setDeviceAndContext(device4Ptr, context3Ptr);
#else
	createBackbuffer(kinc_g4_antialiasing_samples());
	currentRenderTargetViews[0] = renderTargetView;
	currentDepthStencilView = depthStencilView;
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(renderTargetWidth), static_cast<float>(renderTargetHeight));
	context->RSSetViewports(1, &viewPort);
#endif

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

	initSamplers();

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

	kinc_microsoft_affirm(device->CreateBlendState(&blendDesc, &blending));
	context->OMSetBlendState(blending, nullptr, 0xffffffff);
}

void kinc_g4_flush() {}

namespace {
	ID3D11ShaderResourceView* nullviews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {0};
}

static kinc_g4_index_buffer_t *currentIndexBuffer = NULL;

void kinc_internal_g4_index_buffer_set(kinc_g4_index_buffer_t *buffer) {
	currentIndexBuffer = buffer;
	context->IASetIndexBuffer(buffer->impl.ib, DXGI_FORMAT_R32_UINT, 0);
}

void kinc_g4_draw_indexed_vertices() {
	if (currentPipeline->tessellation_control_shader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	context->DrawIndexed(currentIndexBuffer->impl.count, 0, 0);

	context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullviews);
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	kinc_g4_draw_indexed_vertices_from_to_from(start, count, 0);
}

void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_start) {
	if (currentPipeline->tessellation_control_shader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	context->DrawIndexed(count, start, vertex_start);

	context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullviews);
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, 0, currentIndexBuffer->impl.count);
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
	if (currentPipeline->tessellation_control_shader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	context->DrawIndexedInstanced(count, instanceCount, start, 0, 0);
	
	context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullviews);
}

namespace {
	D3D11_TEXTURE_ADDRESS_MODE convertAddressing(kinc_g4_texture_addressing_t addressing) {
		switch (addressing) {
		default:
		case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case KINC_G4_TEXTURE_ADDRESSING_MIRROR:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case KINC_G4_TEXTURE_ADDRESSING_BORDER:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		}
	}
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	if (unit.impl.unit < 0) {
		return;
	}

	switch (dir) {
	case KINC_G4_TEXTURE_DIRECTION_U:
		lastSamplers[unit.impl.unit].AddressU = convertAddressing(addressing);
		break;
	case KINC_G4_TEXTURE_DIRECTION_V:
		lastSamplers[unit.impl.unit].AddressV = convertAddressing(addressing);
		break;
	case KINC_G4_TEXTURE_DIRECTION_W:
		lastSamplers[unit.impl.unit].AddressW = convertAddressing(addressing);
		break;
	}

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.impl.unit]);
	context->PSSetSamplers(unit.impl.unit, 1, &sampler);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	kinc_g4_set_texture_addressing(unit, dir, addressing);
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	const float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f, ((color & 0xff000000) >> 24) / 255.0f};
	for (int i = 0; i < renderTargetCount; ++i) {
		if (currentRenderTargetViews[i] != nullptr && flags & KINC_G4_CLEAR_COLOR) {
			context->ClearRenderTargetView(currentRenderTargetViews[i], clearColor);
		}
	}
	if (currentDepthStencilView != nullptr && (flags & KINC_G4_CLEAR_DEPTH) || (flags & KINC_G4_CLEAR_STENCIL)) {
		unsigned d3dflags = ((flags & KINC_G4_CLEAR_DEPTH) ? D3D11_CLEAR_DEPTH : 0) | ((flags & KINC_G4_CLEAR_STENCIL) ? D3D11_CLEAR_STENCIL : 0);
		context->ClearDepthStencilView(currentDepthStencilView, d3dflags, kinc_clamp(depth, 0.0f, 1.0f), stencil);
	}
}

void kinc_g4_begin(int windowId) {
	if (newRenderTargetWidth != renderTargetWidth || newRenderTargetHeight != renderTargetHeight) {
		depthStencil->Release();
		depthStencilView->Release();
		renderTargetView->Release();
		backBuffer->Release();
		kinc_microsoft_affirm(swapChain->ResizeBuffers(2, newRenderTargetWidth, newRenderTargetHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0));
		createBackbuffer(kinc_g4_antialiasing_samples());
		kinc_g4_restore_render_target();
	}
#ifdef KORE_WINDOWSAPP
	// TODO (DK) do i need to do something here?
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = static_cast<float>(x);
	viewport.TopLeftY = static_cast<float>(y);
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);
}

void kinc_internal_set_rasterizer_state(kinc_g4_pipeline_t *pipeline, bool scissoring);

void kinc_g4_scissor(int x, int y, int width, int height) {
	D3D11_RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	context->RSSetScissorRects(1, &rect);
	kinc_internal_scissoring = true;
	if (currentPipeline != nullptr) {
		kinc_internal_set_rasterizer_state(currentPipeline, kinc_internal_scissoring);
	}
}

void kinc_g4_disable_scissor() {
	context->RSSetScissorRects(0, nullptr);
	kinc_internal_scissoring = false;
	if (currentPipeline != nullptr) {
		kinc_internal_set_rasterizer_state(currentPipeline, kinc_internal_scissoring);
	}
}

void kinc_internal_set_pipeline(kinc_g4_pipeline_t *pipeline, bool scissoring);

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_internal_set_pipeline(pipeline, kinc_internal_scissoring);
}

void kinc_g4_set_stencil_reference_value(int value) {
	if (currentPipeline != nullptr) {
		context->OMSetDepthStencilState(currentPipeline->impl.depthStencilState, value);
	}
}

void kinc_g4_end(int windowId) {}

bool kinc_g4_swap_buffers() {
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

void kinc_g4_set_texture_operation(kinc_g4_texture_operation_t operation, kinc_g4_texture_argument_t arg1, kinc_g4_texture_argument_t arg2) {
	// TODO
}

namespace {
	void setInt(uint8_t* constants, uint32_t offset, uint32_t size, int value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
	}

	void setInt2(uint8_t* constants, uint32_t offset, uint32_t size, int value1, int value2) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value1;
		ints[1] = value2;
	}

	void setInt3(uint8_t* constants, uint32_t offset, uint32_t size, int value1, int value2, int value3) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value1;
		ints[1] = value2;
		ints[2] = value3;
	}

	void setInt4(uint8_t* constants, uint32_t offset, uint32_t size, int value1, int value2, int value3, int value4) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value1;
		ints[1] = value2;
		ints[2] = value3;
		ints[3] = value4;
	}

	void setInts(uint8_t* constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, int* values, int count) {
		if (size == 0) return;
		int *ints = reinterpret_cast<int*>(&constants[offset]);
		if (columns == 4 && rows == 4) {
			for (int i = 0; i < count / 16 && i < static_cast<int>(size) / 4; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						ints[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
					}
				}
			}
		}
		else if (columns == 3 && rows == 3) {
			for (int i = 0; i < count / 9 && i < static_cast<int>(size) / 3; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						ints[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
					}
				}
			}
		}
		else if (columns == 2 && rows == 2) {
			for (int i = 0; i < count / 4 && i < static_cast<int>(size) / 2; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						ints[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
					}
				}
			}
		}
		else {
			for (int i = 0; i < count && i * 4 < static_cast<int>(size); ++i) {
				ints[i] = values[i];
			}
		}
	}

	void setFloat(uint8_t* constants, uint32_t offset, uint32_t size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(uint8_t* constants, uint32_t offset, uint32_t size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(uint8_t* constants, uint32_t offset, uint32_t size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(uint8_t* constants, uint32_t offset, uint32_t size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(uint8_t* constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, float* values, int count) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		if (columns == 4 && rows == 4) {
			for (int i = 0; i < count / 16 && i < static_cast<int>(size) / 4; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
					}
				}
			}
		}
		else if (columns == 3 && rows == 3) {
			for (int i = 0; i < count / 9 && i < static_cast<int>(size) / 3; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
					}
				}
			}
		}
		else if (columns == 2 && rows == 2) {
			for (int i = 0; i < count / 4 && i < static_cast<int>(size) / 2; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
					}
				}
			}
		}
		else {
			for (int i = 0; i < count && i * 4 < static_cast<int>(size); ++i) {
				floats[i] = values[i];
			}
		}
	}

	void setBool(uint8_t* constants, uint32_t offset, uint32_t size, bool value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(uint8_t* constants, uint32_t offset, uint32_t size, kinc_matrix4x4_t *value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value->m[y + x * 4];
			}
		}
	}

	void setMatrix(uint8_t* constants, uint32_t offset, uint32_t size, kinc_matrix3x3_t *value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value->m[y + x * 3];
			}
		}
	}
}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	::setInt(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	::setInt(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	::setInt(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	::setInt(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	::setInt(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {
	::setInt2(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2);
	::setInt2(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2);
	::setInt2(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2);
	::setInt2(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2);
	::setInt2(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2);
}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {
	::setInt3(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3);
	::setInt3(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3);
	::setInt3(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3);
	::setInt3(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3);
	::setInt3(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3);
}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {
	::setInt4(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3, value4);
	::setInt4(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3, value4);
	::setInt4(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3, value4);
	::setInt4(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3, value4);
	::setInt4(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3, value4);
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	::setInts(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, location.impl.vertexColumns, location.impl.vertexRows, values, count);
	::setInts(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, location.impl.fragmentColumns, location.impl.fragmentRows, values,
	            count);
	::setInts(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, location.impl.geometryColumns, location.impl.geometryRows, values,
	            count);
	::setInts(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, location.impl.tessEvalColumns, location.impl.tessEvalRows, values,
	            count);
	::setInts(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, location.impl.tessControlColumns,
	            location.impl.tessControlRows,
	            values,
	            count);
}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	::setFloat(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	::setFloat(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	::setFloat(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	::setFloat(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	::setFloat(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	::setFloat2(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2);
	::setFloat2(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2);
	::setFloat2(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2);
	::setFloat2(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2);
	::setFloat2(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2);
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	::setFloat3(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3);
	::setFloat3(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3);
	::setFloat3(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3);
	::setFloat3(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3);
	::setFloat3(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3);
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	::setFloat4(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3, value4);
	::setFloat4(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3, value4);
	::setFloat4(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3, value4);
	::setFloat4(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3, value4);
	::setFloat4(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3, value4);
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	::setFloats(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, location.impl.vertexColumns, location.impl.vertexRows, values, count);
	::setFloats(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, location.impl.fragmentColumns, location.impl.fragmentRows, values,
	            count);
	::setFloats(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, location.impl.geometryColumns, location.impl.geometryRows, values,
	            count);
	::setFloats(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, location.impl.tessEvalColumns, location.impl.tessEvalRows, values,
	            count);
	::setFloats(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, location.impl.tessControlColumns,
	            location.impl.tessControlRows,
	            values,
	            count);
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	::setBool(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	::setBool(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	::setBool(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	::setBool(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	::setBool(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	::setMatrix(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	::setMatrix(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	::setMatrix(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	::setMatrix(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	::setMatrix(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	::setMatrix(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	::setMatrix(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	::setMatrix(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
}

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
	if (unit.impl.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
	case KINC_G4_TEXTURE_FILTER_LINEAR:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.impl.unit].Filter = d3d11filter;
	lastSamplers[unit.impl.unit].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.impl.unit]);
	context->PSSetSamplers(unit.impl.unit, 1, &sampler);
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_magnification_filter(texunit, filter);
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
	if (unit.impl.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
	case KINC_G4_TEXTURE_FILTER_LINEAR:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.impl.unit].Filter = d3d11filter;
	lastSamplers[unit.impl.unit].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.impl.unit]);
	context->PSSetSamplers(unit.impl.unit, 1, &sampler);
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_minification_filter(texunit, filter);
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {
	if (unit.impl.unit < 0) return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_MIPMAP_FILTER_NONE:
	case KINC_G4_MIPMAP_FILTER_POINT:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_ANISOTROPIC;
			break;
		}
		break;
	case KINC_G4_MIPMAP_FILTER_LINEAR:
		switch (lastSamplers[unit.impl.unit].Filter) {
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
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_ANISOTROPIC;
			break;
		}
		break;
	}

	lastSamplers[unit.impl.unit].Filter = d3d11filter;
	lastSamplers[unit.impl.unit].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.impl.unit]);
	context->PSSetSamplers(unit.impl.unit, 1, &sampler);
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	kinc_g4_set_texture_mipmap_filter(texunit, filter);
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {
	if (unit.impl.unit < 0) return;

	if (enabled) {
		lastSamplers[unit.impl.unit].ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		lastSamplers[unit.impl.unit].Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	}
	else {
		lastSamplers[unit.impl.unit].ComparisonFunc = D3D11_COMPARISON_NEVER;
	}

	ID3D11SamplerState* sampler = getSamplerState(lastSamplers[unit.impl.unit]);
	context->PSSetSamplers(unit.impl.unit, 1, &sampler);
}

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {
	kinc_g4_set_texture_compare_mode(unit, enabled);
}

bool kinc_g4_render_targets_inverted_y() {
	return false;
}

bool kinc_g4_non_pow2_textures_supported() {
	return true;
}

void kinc_g4_restore_render_target() {
	currentRenderTargetViews[0] = renderTargetView;
	currentDepthStencilView = depthStencilView;
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	renderTargetCount = 1;
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(renderTargetWidth), static_cast<float>(renderTargetHeight));
	context->RSSetViewports(1, &viewPort);
}

void kinc_g4_set_render_targets(kinc_g4_render_target **targets, int count) {
	currentDepthStencilView = targets[0]->impl.depthStencilView[0];

	renderTargetCount = count;
	for (int i = 0; i < count; ++i) {
		currentRenderTargetViews[i] = targets[i]->impl.renderTargetViewRender[0];
	}

	context->OMSetRenderTargets(count, currentRenderTargetViews, currentDepthStencilView);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(targets[0]->width), static_cast<float>(targets[0]->height));
	context->RSSetViewports(1, &viewPort);
}

void kinc_g4_set_render_target_face(kinc_g4_render_target *texture, int face) {
	renderTargetCount = 1;
	currentRenderTargetViews[0] = texture->impl.renderTargetViewRender[face];
	currentDepthStencilView = texture->impl.depthStencilView[face];
	context->OMSetRenderTargets(1, currentRenderTargetViews, currentDepthStencilView);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(texture->width), static_cast<float>(texture->height));
	context->RSSetViewports(1, &viewPort);
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count) {
	kinc_internal_g4_vertex_buffer_set(buffers[0], 0);

	ID3D11Buffer** d3dbuffers = (ID3D11Buffer**)alloca(count * sizeof(ID3D11Buffer*));
	for (int i = 0; i < count; ++i) {
		d3dbuffers[i] = buffers[i]->impl.vb;
	}

	UINT* strides = (UINT*)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		strides[i] = buffers[i]->impl.stride;
	}

	UINT* internaloffsets = (UINT*)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		internaloffsets[i] = 0;
	}

	context->IASetVertexBuffers(0, count, d3dbuffers, strides, internaloffsets);
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_internal_g4_index_buffer_set(buffer);
}

void kinc_internal_texture_set(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	kinc_internal_texture_set(texture, unit);
}

void kinc_internal_texture_set_image(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	kinc_internal_texture_set_image(texture, unit);
}

static unsigned queryCount = 0;
static std::vector<ID3D11Query*> queryPool;

bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery) {
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_OCCLUSION;
	queryDesc.MiscFlags = 0;
	ID3D11Query* pQuery = nullptr;
	HRESULT result = device->CreateQuery(&queryDesc, &pQuery);

	if (FAILED(result)) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Internal query creation failed, result: 0x%X.", result);
		return false;
	}

	queryPool.push_back(pQuery);
	*occlusionQuery = queryCount;
	++queryCount;

	return true;
}

void kinc_g4_delete_occlusion_query(unsigned occlusionQuery) {
	if (occlusionQuery < queryPool.size()) queryPool[occlusionQuery] = nullptr;
}

void kinc_g4_start_occlusion_query(unsigned occlusionQuery) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		context->Begin(pQuery);
	}
}

void kinc_g4_end_occlusion_query(unsigned occlusionQuery) {
	ID3D11Query *pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		context->End(pQuery);
	}
}

bool kinc_g4_are_query_results_available(unsigned occlusionQuery) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		if (S_OK == context->GetData(pQuery, 0, 0, 0)) return true;
	}
	return false;
}

void kinc_g4_get_query_results(unsigned occlusionQuery, unsigned *pixelCount) {
	ID3D11Query* pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		UINT64 numberOfPixelsDrawn;
		HRESULT result = context->GetData(pQuery, &numberOfPixelsDrawn, sizeof(UINT64), 0);
		if (S_OK == result) {
			*pixelCount = static_cast<unsigned>(numberOfPixelsDrawn);
		}
		else {
			kinc_log(KINC_LOG_LEVEL_INFO, "Check first if results are available");
			*pixelCount = 0;
		}
	}
}

void kinc_internal_texture_array_set(kinc_g4_texture_array_t *array, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, kinc_g4_texture_array_t *array) {
	kinc_internal_texture_array_set(array, unit);
}

extern "C" void kinc_internal_resize(int window, int width, int height) {
	newRenderTargetWidth = width;
	newRenderTargetHeight = height;
}

extern "C" void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {}

extern "C" bool kinc_window_vsynced(int window_index) {
	return vsync;
}
