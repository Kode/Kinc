#include "pch.h"

#include "Direct3D12.h"
#include "IndexBuffer5Impl.h"
#include "PipelineState5Impl.h"
#include "VertexBuffer5Impl.h"
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/Math/Core.h>
#ifdef KORE_WINDOWS
#include <dxgi1_4.h>
#undef CreateWindow
#endif
#include <Kore/System.h>
#include <Kore/WinError.h>
#include <wrl.h>

/*IDXGIFactory4* dxgiFactory;
ID3D12Device* device;
ID3D12GraphicsCommandList* commandList;
ID3D12CommandQueue* commandQueue;
ID3D12CommandAllocator* commandAllocators[frameCount];
unsigned currentFrame = 0;
ID3D12Fence* fence;
UINT64 fenceValues[frameCount];
HANDLE fenceEvent;
ID3D12Resource* renderTargets[frameCount];
ID3D12DescriptorHeap* rtvHeap;
unsigned rtvDescriptorSize;
ID3D12CommandAllocator* bundleAllocator;
ID3D12RootSignature* rootSignature;
D3D12_VIEWPORT screenViewport;
D3D12_RECT scissorRect;
ID3D12DescriptorHeap* cbvHeap;*/
// ID3D12DeviceContext* context;
// ID3D12RenderTargetView* renderTargetView;
// ID3D12DepthStencilView* depthStencilView;

int currentBackBuffer = 0;
ID3D12Device* device;
ID3D12RootSignature* rootSignature;
//ID3D12GraphicsCommandList* commandList;
ID3D12Resource* depthStencilTexture;
ID3D12CommandQueue* commandQueue;

int renderTargetWidth;
int renderTargetHeight;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];
Kore::u8 geometryConstants[1024 * 4];
Kore::u8 tessControlConstants[1024 * 4];
Kore::u8 tessEvalConstants[1024 * 4];

using namespace Kore;

#ifndef KORE_WINDOWS
#define DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC1
#define IDXGISwapChain IDXGISwapChain1
#endif

struct RenderEnvironment {
	ID3D12Device* device;
	ID3D12CommandQueue* queue;
	IDXGISwapChain* swapChain;
};

#ifndef KORE_WINDOWS
void createSwapChain(RenderEnvironment* env, IDXGIAdapter* adapter, const DXGI_SWAP_CHAIN_DESC1* desc);
#endif

namespace {
	D3D12_VIEWPORT viewport;
	D3D12_RECT rectScissor;
	ID3D12Resource* renderTarget;
	ID3D12DescriptorHeap* renderTargetDescriptorHeap;
	ID3D12DescriptorHeap* depthStencilDescriptorHeap;
	UINT64 currentFenceValue;
	UINT64 fenceValues[QUEUE_SLOT_COUNT];
	HANDLE frameFenceEvents[QUEUE_SLOT_COUNT];
	ID3D12Fence* frameFences[QUEUE_SLOT_COUNT];
	IDXGISwapChain* swapChain;
	ID3D12Fence* uploadFence;
	ID3D12GraphicsCommandList* initCommandList;
	ID3D12CommandAllocator* initCommandAllocator;

	RenderEnvironment createDeviceAndSwapChainHelper(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL minimumFeatureLevel, const DXGI_SWAP_CHAIN_DESC* swapChainDesc) {
		RenderEnvironment result;
#ifdef KORE_WINDOWS
		affirm(D3D12CreateDevice(adapter, minimumFeatureLevel, IID_PPV_ARGS(&result.device)));

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		affirm(result.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&result.queue)));

		IDXGIFactory4* dxgiFactory;
		affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

		DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
		affirm(dxgiFactory->CreateSwapChain(result.queue, &swapChainDescCopy, &result.swapChain));
#else
		createSwapChain(&result, adapter, swapChainDesc);
#endif
		return result;
	}

	void waitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void createRenderTargetView() {
		const D3D12_RESOURCE_DESC resourceDesc = renderTarget->GetDesc();

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(renderTarget, &viewDesc, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void setupSwapChain() {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&renderTargetDescriptorHeap));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(&depthStencilDescriptorHeap));

		CD3DX12_RESOURCE_DESC depthTexture(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
			renderTargetWidth, renderTargetHeight, 1, 1,
			DXGI_FORMAT_D32_FLOAT, 1, 0, D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
			IID_GRAPHICS_PPV_ARGS(&depthStencilTexture));

		device->CreateDepthStencilView(depthStencilTexture, nullptr, depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		currentFenceValue = 0;

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fenceValues[i] = 0;
			device->CreateFence(currentFenceValue, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&frameFences[i]));
		}

		swapChain->GetBuffer(currentBackBuffer, IID_GRAPHICS_PPV_ARGS(&renderTarget));
		createRenderTargetView();
	}

	void createDeviceAndSwapChain(int width, int height, HWND window) {
#ifdef _DEBUG
		ID3D12Debug* debugController;
		D3D12GetDebugInterface(IID_GRAPHICS_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
#endif

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

#ifdef KORE_WINDOWS
		swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.OutputWindow = window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = true;
#else
		initSwapChain(&swapChainDesc, width, height, window);
#endif

		auto renderEnv = createDeviceAndSwapChainHelper(nullptr, D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

		device = renderEnv.device;
		commandQueue = renderEnv.queue;
		swapChain = renderEnv.swapChain;

		setupSwapChain();
	}

	void createViewportScissor(int width, int height) {
		rectScissor = {0, 0, width, height};
		viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
	}

	void createRootSignature() {
		const int textureCount = 16;

		ID3DBlob* rootBlob;
		ID3DBlob* errorBlob;

		CD3DX12_ROOT_PARAMETER parameters[3];

		CD3DX12_DESCRIPTOR_RANGE range{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)textureCount, 0};
		parameters[0].InitAsDescriptorTable(1, &range);

		parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		parameters[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC samplers[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			samplers[i].Init(i, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
		}

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(3, parameters, textureCount, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
		device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(&rootSignature));
	}

	void initialize(int width, int height, HWND window) {
		createDeviceAndSwapChain(width, height, window);
		createViewportScissor(width, height);
		createRootSignature();

		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&uploadFence));

		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&initCommandAllocator));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, nullptr, IID_GRAPHICS_PPV_ARGS(&initCommandList));

		initCommandList->Close();

		ID3D12CommandList* commandLists[] = {initCommandList};
		commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
		commandQueue->Signal(uploadFence, 1);

		HANDLE waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		waitForFence(uploadFence, 1, waitEvent);

		initCommandAllocator->Reset();
		initCommandList->Release();      // check me
		initCommandAllocator->Release(); // check me

		CloseHandle(waitEvent);
	}

	void shutdown() {
		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			waitForFence(frameFences[i], fenceValues[i], frameFenceEvents[i]);
		}

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			CloseHandle(frameFenceEvents[i]);
		}
	}
}

namespace Kore {
	extern PipelineState5Impl* currentProgram;
}

namespace {
	unsigned hz;
	bool vsync;

	// D3D_FEATURE_LEVEL featureLevel;
	// ID3D11DepthStencilState* depthTestState = nullptr;
	// ID3D11DepthStencilState* noDepthTestState = nullptr;

	/*IDXGISwapChain1* swapChain;

	void waitForGpu() {
	    affirm(commandQueue->Signal(fence, fenceValues[currentFrame]));
	    affirm(fence->SetEventOnCompletion(fenceValues[currentFrame], fenceEvent));
	    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
	    fenceValues[currentFrame]++;
	}*/
}

void Graphics5::destroy(int window) {}

void Graphics5::init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

	HWND hwnd = (HWND)System::windowHandle(window);
	renderTargetWidth = System::windowWidth(window);
	renderTargetHeight = System::windowHeight(window);
	initialize(renderTargetWidth, renderTargetHeight, hwnd);

#ifdef KORE_WINDOWS
	if (System::hasShowWindowFlag()) {
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);
	}
#endif

	begin(window);
}

void Graphics5::changeResolution(int width, int height) {}

void Graphics5::setup() {}

void Graphics5::makeCurrent(int window) {}

void Graphics5::clearCurrent() {}

//void* Graphics::getControl() {
//	return nullptr;
//}

/*void Graphics5::drawIndexedVertices() {
	// Program::setConstants();
	// context->DrawIndexed(IndexBuffer::_current->count(), 0, 0);

	drawIndexedVertices(0, IndexBuffer::_current->myCount);
}

void Graphics5::drawIndexedVertices(int start, int count) {
	//commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&VertexBuffer::_current->view);
	//commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&IndexBuffer::_current->view);
	//commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);

	PipelineState5Impl::setConstants();

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&VertexBuffer::_current->view);
	commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&IndexBuffer::_current->indexBufferView);

	u8* data;
	vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->Map(0, nullptr, (void**)&data);
	memcpy(data, vertexConstants, sizeof(vertexConstants));
	vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->Unmap(0, nullptr);

	fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->Map(0, nullptr, (void**)&data);
	memcpy(data, fragmentConstants, sizeof(fragmentConstants));
	fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->Unmap(0, nullptr);

	commandList->SetGraphicsRootConstantBufferView(1, vertexConstantBuffers[currentBackBuffer * 128 + currentInstance]->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(2, fragmentConstantBuffers[currentBackBuffer * 128 + currentInstance]->GetGPUVirtualAddress());
	if (++currentInstance >= 128) currentInstance = 0;

	commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);

}
*/
void Graphics5::drawIndexedVerticesInstanced(int instanceCount) {}

void Graphics5::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {}

void Graphics5::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {}

// (DK) fancy macro's to generate a clickable warning message in visual studio, can be removed when setColorMask() is implemented
#define Stringize(L) #L
#define MakeString(M, L) M(L)
#define $Line MakeString(Stringize, __LINE__)
#define Warning __FILE__ "(" $Line ") : warning: "

void Graphics5::clear(uint flags, uint color, float depth, int stencil) {}

namespace {
	bool began = false;
}

void Graphics5::begin(int window) {
	if (began) return;
	began = true;
#ifdef KORE_WINDOWSAPP
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif

	waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

	//static const float clearColor[] = {0.042f, 0.042f, 0.042f, 1};

	//commandList->ClearRenderTargetView(renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	//commandList->ClearDepthStencilView(depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;
}

void Graphics5::end(int window) {
	began = false;
}

bool Graphics5::vsynced() {
	return vsync;
}

unsigned Graphics5::refreshRate() {
	return hz;
}

bool Graphics5::swapBuffers(int window) {
	swapChain->Present(1, 0);

	currentBackBuffer = (currentBackBuffer + 1) % QUEUE_SLOT_COUNT;
	swapChain->GetBuffer(currentBackBuffer, IID_GRAPHICS_PPV_ARGS(&renderTarget));

	createRenderTargetView();

	const UINT64 fenceValue = currentFenceValue;
	commandQueue->Signal(frameFences[currentBackBuffer], fenceValue);
	fenceValues[currentBackBuffer] = fenceValue;
	++currentFenceValue;

	return true;
}

void Graphics5::flush() {}

void Graphics5::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {}

namespace {
	void setInt(u8* constants, u32 offset, u32 size, int value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(u8* constants, u32 offset, u32 size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, u32 offset, u32 size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, u32 offset, u32 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, u32 offset, u32 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(u8* constants, u32 offset, u32 size, float* values, int count) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}

	void setBool(u8* constants, u32 offset, u32 size, bool value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat4& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat3& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}
}

void Graphics5::setInt(ConstantLocation location, int value) {
	::setInt(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setInt(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setInt(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setInt(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setInt(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics5::setFloat(ConstantLocation location, float value) {
	::setFloat(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setFloat(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setFloat(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setFloat(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setFloat(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics5::setFloat2(ConstantLocation location, float value1, float value2) {
	::setFloat2(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2);
	::setFloat2(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2);
	::setFloat2(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2);
	::setFloat2(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2);
	::setFloat2(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2);
}

void Graphics5::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	::setFloat3(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3);
	::setFloat3(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3);
	::setFloat3(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3);
	::setFloat3(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3);
	::setFloat3(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3);
}

void Graphics5::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	::setFloat4(vertexConstants, location.vertexOffset, location.vertexSize, value1, value2, value3, value4);
	::setFloat4(fragmentConstants, location.fragmentOffset, location.fragmentSize, value1, value2, value3, value4);
	::setFloat4(geometryConstants, location.geometryOffset, location.geometrySize, value1, value2, value3, value4);
	::setFloat4(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value1, value2, value3, value4);
	::setFloat4(tessControlConstants, location.tessControlOffset, location.tessControlSize, value1, value2, value3, value4);
}

void Graphics5::setFloats(ConstantLocation location, float* values, int count) {
	::setFloats(vertexConstants, location.vertexOffset, location.vertexSize, values, count);
	::setFloats(fragmentConstants, location.fragmentOffset, location.fragmentSize, values, count);
	::setFloats(geometryConstants, location.geometryOffset, location.geometrySize, values, count);
	::setFloats(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, values, count);
	::setFloats(tessControlConstants, location.tessControlOffset, location.tessControlSize, values, count);
}

void Graphics5::setBool(ConstantLocation location, bool value) {
	::setBool(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setBool(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setBool(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setBool(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setBool(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics5::setMatrix(ConstantLocation location, const mat4& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics5::setMatrix(ConstantLocation location, const mat3& value) {
	::setMatrix(vertexConstants, location.vertexOffset, location.vertexSize, value);
	::setMatrix(fragmentConstants, location.fragmentOffset, location.fragmentSize, value);
	::setMatrix(geometryConstants, location.geometryOffset, location.geometrySize, value);
	::setMatrix(tessEvalConstants, location.tessEvalOffset, location.tessEvalSize, value);
	::setMatrix(tessControlConstants, location.tessControlOffset, location.tessControlSize, value);
}

void Graphics5::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics5::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

bool Graphics5::renderTargetsInvertedY() {
	return false;
}

bool Graphics5::nonPow2TexturesSupported() {
	return true;
}

void Graphics5::setRenderTargetFace(RenderTarget* texture, int face) {
	
}
/*
void Graphics5::setVertexBuffers(VertexBuffer** buffers, int count) {
	buffers[0]->_set(0);
}

void Graphics5::setIndexBuffer(IndexBuffer& buffer) {
	buffer._set();
}
*/
void Graphics5::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

bool Graphics5::initOcclusionQuery(uint* occlusionQuery) {
	return false;
}

void Graphics5::setImageTexture(TextureUnit unit, Texture* texture) {
	
}

void Graphics5::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics5::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics5::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics5::getQueryResults(uint occlusionQuery, uint* pixelCount) {}

/*void Graphics5::setPipeline(PipelineState* pipeline) {
	pipeline->set(pipeline);
}*/
