#include "pch.h"
#include <Kore/Math/Core.h>
#include "Direct3D11.h"
//#include <Kore/Application.h>
#include "IndexBufferImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Shader.h>
#undef CreateWindow
#include <Kore/System.h>
#include <Kore/WinError.h>
#ifdef SYS_WINDOWSAPP
#include <d3d11_1.h>
#include <wrl.h>
#endif

ID3D11Device* device;
ID3D11DeviceContext* context;
ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;

int renderTargetWidth = 4096;
int renderTargetHeight = 4096;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];
Kore::u8 geometryConstants[1024 * 4];
Kore::u8 tessControlConstants[1024 * 4];
Kore::u8 tessEvalConstants[1024 * 4];

using namespace Kore;

#ifdef SYS_WINDOWSAPP
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
	ID3D11DepthStencilState* depthTestState = nullptr;
	ID3D11DepthStencilState* noDepthTestState = nullptr;
#ifdef SYS_WINDOWSAPP
	IDXGISwapChain1* swapChain;
#else
	IDXGISwapChain* swapChain;
#endif
}

void Graphics::destroy(int windowId) {

}

void Graphics::init(int windowId, int depthBufferBits, int stencilBufferBits) {
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

	HWND hwnd = (HWND)System::windowHandle(windowId);

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
#ifdef SYS_WINDOWSAPP
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
#ifdef SYS_WINDOWSAPP
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
		swapChainDesc.BufferDesc.Width = System::windowWidth(windowId);                                     // use automatic sizing
		swapChainDesc.BufferDesc.Height = System::windowHeight(windowId);
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
		swapChainDesc.OutputWindow = (HWND)System::windowHandle(windowId);
		swapChainDesc.Windowed = true;
#endif

#ifdef SYS_WINDOWSAPP
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
		UINT flags = 0;
#ifdef _DEBUG
		flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
		affirm(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 6, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &context));
#endif	
	}

	ID3D11Texture2D* backBuffer;
	affirm(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));

	affirm(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));
	
	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBuffer->GetDesc(&backBufferDesc);
	renderTargetWidth  = backBufferDesc.Width;
	renderTargetHeight = backBufferDesc.Height;

	// TODO (DK) map depth/stencilBufferBits arguments
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
#ifdef SYS_WINDOWSAPP
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
	if (System::hasShowWindowFlag()) {
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
	}
#endif

	System::makeCurrent(windowId);
}

void Graphics::makeCurrent( int contextId ) {
	// TODO (DK) implement me
}

void Graphics::clearCurrent() {
	// TODO (DK) implement me
}

void Graphics::flush() {

}

void Graphics::changeResolution(int width, int height) {

}

void Graphics::drawIndexedVertices() {
	if (currentProgram->tessControlShader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	Program::setConstants();
	context->DrawIndexed(IndexBuffer::_current->count(), 0, 0);
}

void Graphics::drawIndexedVertices(int start, int count) {
	if (currentProgram->tessControlShader != nullptr) {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	Program::setConstants();
	context->DrawIndexed(count, start, 0);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {

}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {

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

// (DK) fancy macro's to generate a clickable warning message in visual studio, can be removed when setColorMask() is implemented
#define Stringize( L )			#L
#define MakeString( M, L )		M(L)
#define $Line					\
	MakeString( Stringize, __LINE__ )
#define Warning				\
	__FILE__ "(" $Line ") : warning: "

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {
#pragma message(Warning "(DK) Robert, please implement d3d11's version of setColorMask() here")
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

void Graphics::begin(int windowId) {	
#ifdef SYS_WINDOWSAPP
	// TODO (DK) do i need to do something here?
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif
}

void Graphics::viewport(int x, int y, int width, int height) {
	//TODO
}

void Graphics::scissor(int x, int y, int width, int height) {
	//TODO
}

void Graphics::disableScissor() {
	//TODO
}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {
	//TODO
}

void Graphics::end(int windowId) {
	
}

bool Graphics::vsynced() {
	return vsync;
}

unsigned Graphics::refreshRate() {
	return hz;
}

void Graphics::swapBuffers(int windowId) {
	HRESULT hr = swapChain->Present(1, 0);
	// TODO: if (hr == DXGI_STATUS_OCCLUDED)...
	// http://www.pouet.net/topic.php?which=10454
	// "Proper handling of DXGI_STATUS_OCCLUDED would be to pause the application,
	// and periodically call Present with the TEST flag, and when it returns S_OK, resume rendering."

	//if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
	//	Initialize(m_window);
	//}
	//else {
		affirm(SUCCEEDED(hr));
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

void Graphics::setInt(ConstantLocation location, int value) {
	::setInt(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setInt(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setInt(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setInt(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setInt(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics::setFloat(ConstantLocation location, float value) {
	::setFloat(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setFloat(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setFloat(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setFloat(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setFloat(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {
	::setFloat2(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2);
	::setFloat2(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2);
	::setFloat2(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2);
	::setFloat2(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2);
	::setFloat2(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2);
}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	::setFloat3(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3);
	::setFloat3(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3);
	::setFloat3(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3);
	::setFloat3(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3);
	::setFloat3(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3);
}

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	::setFloat4(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3, value4);
	::setFloat4(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3, value4);
	::setFloat4(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3, value4);
	::setFloat4(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3, value4);
	::setFloat4(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3, value4);
}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {
	::setFloats(vertexConstants, location.vertexOffset, location.vertexSize, values, count);
	::setFloats(fragmentConstants, location.fragmentOffset, location.fragmentSize, values, count);
	::setFloats(geometryConstants, location.geometryOffset, location.geometrySize, values, count);
	::setFloats(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, values, count);
	::setFloats(tessControlConstants, location.tessControlOffset, location.tessControlSize, values, count);
}

void Graphics::setBool(ConstantLocation location, bool value) {
	::setBool(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setBool(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setBool(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setBool(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setBool(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {

}

namespace {
	D3D11_BLEND convert(BlendingOperation operation) {
		switch (operation) {
		case BlendOne:
			return D3D11_BLEND_ONE;
		case BlendZero:
			return D3D11_BLEND_ZERO;
		case SourceAlpha:
			return D3D11_BLEND_SRC_ALPHA;
		case DestinationAlpha:
			return D3D11_BLEND_DEST_ALPHA;
		case InverseSourceAlpha:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case InverseDestinationAlpha:
			return D3D11_BLEND_INV_DEST_ALPHA;
		default:
			//	throw Exception("Unknown blending operation.");
			return D3D11_BLEND_SRC_ALPHA;
		}
	}
}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	ID3D11BlendState* blendState = nullptr;
	
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	ZeroMemory(&rtbd, sizeof(rtbd));

	rtbd.BlendEnable = source != BlendOne || destination != BlendZero;
	rtbd.SrcBlend = convert(source);
	rtbd.DestBlend = convert(destination);
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = convert(source);
	rtbd.DestBlendAlpha = convert(destination);
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	device->CreateBlendState(&blendDesc, &blendState);
	
	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	context->OMSetBlendState(blendState, blendFactor, sampleMask);
}

bool Graphics::renderTargetsInvertedY() {
	return false;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::restoreRenderTarget() {
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(renderTargetWidth), static_cast<float>(renderTargetHeight));
	context->RSSetViewports(1, &viewPort);
}

void Graphics::setRenderTarget(RenderTarget* target, int num, int additionalTargets) {
	context->OMSetRenderTargets(1, &target->renderTargetView, nullptr);
	CD3D11_VIEWPORT viewPort(0.0f, 0.0f, static_cast<float>(target->width), static_cast<float>(target->height));
	context->RSSetViewports(1, &viewPort);
}

void Graphics::setVertexBuffers(VertexBuffer** buffers, int count) {
	buffers[0]->_set(0);
}

void Graphics::setIndexBuffer(IndexBuffer& buffer) {
	buffer._set();
}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

void Graphics::setup() {

}
