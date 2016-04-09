#include "pch.h"
#include <Kore/Math/Core.h>
#include <dxgi1_4.h>
#include "Direct3D12.h"
#include <Kore/Application.h>
#include "IndexBufferImpl.h"
#include "VertexBufferImpl.h"
#include "ProgramImpl.h"
#include <Kore/Graphics/Shader.h>
#undef CreateWindow
#include <Kore/System.h>
#include <Kore/WinError.h>
#include <wrl.h>
#include "d3dx12.h"

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
//ID3D12DeviceContext* context;
//ID3D12RenderTargetView* renderTargetView;
//ID3D12DepthStencilView* depthStencilView;

int currentBackBuffer = 0;
ID3D12Device* device;
ID3D12RootSignature* rootSignature;
ID3D12GraphicsCommandList* commandList;
ID3D12Resource* constantBuffers[QUEUE_SLOT_COUNT];

int renderTargetWidth;
int renderTargetHeight;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];
Kore::u8 geometryConstants[1024 * 4];
Kore::u8 tessControlConstants[1024 * 4];
Kore::u8 tessEvalConstants[1024 * 4];

using namespace Kore;

namespace {
	ID3D12CommandAllocator* commandAllocators[QUEUE_SLOT_COUNT];
	ID3D12GraphicsCommandList* commandLists[QUEUE_SLOT_COUNT];
	D3D12_VIEWPORT viewport;
	D3D12_RECT rectScissor;
	ID3D12Resource* renderTarget;
	ID3D12DescriptorHeap* renderTargetDescriptorHeap;
	ID3D12CommandQueue* commandQueue;
	UINT64 currentFenceValue;
	UINT64 fenceValues[QUEUE_SLOT_COUNT];
	HANDLE frameFenceEvents[QUEUE_SLOT_COUNT];
	ID3D12Fence* frameFences[QUEUE_SLOT_COUNT];
	IDXGISwapChain* swapChain;
	ID3D12Fence* uploadFence;
	ID3D12GraphicsCommandList* initCommandList;
	ID3D12CommandAllocator* initCommandAllocator;	

	struct RenderEnvironment {
		ID3D12Device* device;
		ID3D12CommandQueue* queue;
		IDXGISwapChain* swapChain;
	};

	RenderEnvironment createDeviceAndSwapChainHelper(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL minimumFeatureLevel, const DXGI_SWAP_CHAIN_DESC* swapChainDesc) {
		RenderEnvironment result;

		affirm(D3D12CreateDevice(adapter, minimumFeatureLevel, IID_PPV_ARGS(&result.device)));

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		affirm(result.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&result.queue)));

		IDXGIFactory4* dxgiFactory;
		affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
		
		DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
		affirm(dxgiFactory->CreateSwapChain(result.queue, &swapChainDescCopy, &result.swapChain));

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
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&renderTargetDescriptorHeap));

		currentFenceValue = 0;

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fenceValues[i] = 0;
			device->CreateFence(currentFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFences[i]));
		}

		swapChain->GetBuffer(currentBackBuffer, IID_PPV_ARGS(&renderTarget));
		createRenderTargetView();
	}

	void createDeviceAndSwapChain(int width, int height, HWND window) {
#ifdef _DEBUG
		ID3D12Debug* debugController;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
		debugController->EnableDebugLayer();
#endif

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.OutputWindow = window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = true;

		auto renderEnv = createDeviceAndSwapChainHelper(nullptr, D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

		device = renderEnv.device;
		commandQueue = renderEnv.queue;
		swapChain = renderEnv.swapChain;

		setupSwapChain();
	}

	void createAllocatorsAndCommandLists() {
		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[i], nullptr, IID_PPV_ARGS(&commandLists[i]));
			commandLists[i]->Close();
		}
	}

	void createViewportScissor(int width, int height) {
		rectScissor = { 0, 0, width, height };
		viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	}

	void createRootSignature() {
		const int textureCount = 16;

		ID3DBlob* rootBlob;
		ID3DBlob* errorBlob;

		CD3DX12_ROOT_PARAMETER parameters[2];

		CD3DX12_DESCRIPTOR_RANGE range{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)textureCount, 0 };
		parameters[0].InitAsDescriptorTable(1, &range);

		parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

		CD3DX12_STATIC_SAMPLER_DESC samplers[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			samplers[i].Init(i, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
		}

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(2, parameters, textureCount, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
		device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	}

	void createConstantBuffer() {
		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexConstants) + sizeof(fragmentConstants)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffers[i]));

			void* p;
			constantBuffers[i]->Map(0, nullptr, &p);
			ZeroMemory(p, sizeof(vertexConstants) + sizeof(fragmentConstants));
			constantBuffers[i]->Unmap(0, nullptr);
		}
	}

	void initialize(int width, int height, HWND window) {
		createDeviceAndSwapChain(width, height, window);
		createAllocatorsAndCommandLists();
		createViewportScissor(width, height);
		createRootSignature();

		createConstantBuffer();

		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&initCommandAllocator));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, nullptr, IID_PPV_ARGS(&initCommandList));

		initCommandList->Close();

		ID3D12CommandList* commandLists[] = { initCommandList };
		commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
		commandQueue->Signal(uploadFence, 1);

		HANDLE waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		waitForFence(uploadFence, 1, waitEvent);

		initCommandAllocator->Reset();
		initCommandList->Release(); // check me
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
	extern ProgramImpl* currentProgram;
}

namespace {
	unsigned hz;
	bool vsync;

	//D3D_FEATURE_LEVEL featureLevel;
	//ID3D11DepthStencilState* depthTestState = nullptr;
	//ID3D11DepthStencilState* noDepthTestState = nullptr;

	/*IDXGISwapChain1* swapChain;

	void waitForGpu() {
		affirm(commandQueue->Signal(fence, fenceValues[currentFrame]));
		affirm(fence->SetEventOnCompletion(fenceValues[currentFrame], fenceEvent));
		WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
		fenceValues[currentFrame]++;
	}*/
}

void Graphics::destroy() {

}

void Graphics::init() {
	for (int i = 0; i < 1024 * 4; ++i) vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i) fragmentConstants[i] = 0;

	HWND hwnd = (HWND)System::createWindow();
	renderTargetWidth = Application::the()->width();
	renderTargetHeight = Application::the()->height();
	initialize(renderTargetWidth, renderTargetHeight, hwnd);

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
	//Program::setConstants();
	//context->DrawIndexed(IndexBuffer::_current->count(), 0, 0);
	
	drawIndexedVertices(0, IndexBuffer::_current->myCount);
}

void Graphics::drawIndexedVertices(int start, int count) {
	/*commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&VertexBuffer::_current->view);
	commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&IndexBuffer::_current->view);
	commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);*/

	Program::setConstants();

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&VertexBuffer::_current->view);
	commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&IndexBuffer::_current->indexBufferView);
	commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {

}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {

}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	
}

// (DK) fancy macro's to generate a clickable warning message in visual studio, can be removed when setColorMask() is implemented
#define Stringize( L )			#L
#define MakeString( M, L )		M(L)
#define $Line					\
	MakeString( Stringize, __LINE__ )
#define Warning				\
	__FILE__ "(" $Line ") : warning: "

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {
#pragma message(Warning "(DK) Robert, please implement d3d12's version of setColorMask() here")
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {
	
}

void Graphics::begin() {
#ifdef SYS_WINDOWSRT
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif

	waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

	commandAllocators[currentBackBuffer]->Reset();

	commandList = commandLists[currentBackBuffer];
	commandList->Reset(commandAllocators[currentBackBuffer], nullptr);
	commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	commandList->RSSetViewports(1, &::viewport);
	commandList->RSSetScissorRects(1, &rectScissor);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	static const float clearColor[] = { 0.042f, 0.042f, 0.042f, 1 };

	commandList->ClearRenderTargetView(renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;

	commandList = commandLists[currentBackBuffer];
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

void Graphics::end() {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* commandList = commandLists[currentBackBuffer];
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
}

void graphicsFlushAndWait() {
	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);

	const UINT64 fenceValue = currentFenceValue;
	commandQueue->Signal(frameFences[currentBackBuffer], fenceValue);
	fenceValues[currentBackBuffer] = fenceValue;
	++currentFenceValue;

	waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

	commandList->Reset(commandAllocators[currentBackBuffer], nullptr);
	commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &rectScissor);
}

bool Graphics::vsynced() {
	return vsync;
}

unsigned Graphics::refreshRate() {
	return hz;
}

void Graphics::swapBuffers() {
	swapChain->Present(1, 0);

	currentBackBuffer = (currentBackBuffer + 1) % QUEUE_SLOT_COUNT;
	swapChain->GetBuffer(currentBackBuffer, IID_PPV_ARGS(&renderTarget));

	createRenderTargetView();

	const UINT64 fenceValue = currentFenceValue;
	commandQueue->Signal(frameFences[currentBackBuffer], fenceValue);
	fenceValues[currentBackBuffer] = fenceValue;
	++currentFenceValue;
}

void Graphics::flush() {

}

void Graphics::setRenderState(RenderState state, bool on) {
	
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

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {

}

bool Graphics::renderTargetsInvertedY() {
	return false;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::restoreRenderTarget() {
	commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	commandList->RSSetViewports(1, &::viewport);
	commandList->RSSetScissorRects(1, &rectScissor);
}

void Graphics::setRenderTarget(RenderTarget* target, int num, int additionalTargets) {
	commandList->OMSetRenderTargets(1, &target->renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	commandList->RSSetViewports(1, (D3D12_VIEWPORT*)&target->viewport);
	commandList->RSSetScissorRects(1, (D3D12_RECT*)&target->scissor);
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
