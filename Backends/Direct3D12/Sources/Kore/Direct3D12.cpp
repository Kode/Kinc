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

ID3D12Device* device_;
ID3D12RootSignature* rootSignature_;
ID3D12GraphicsCommandList* commandList;

int renderTargetWidth;
int renderTargetHeight;

Kore::u8 vertexConstants[1024 * 4];
Kore::u8 fragmentConstants[1024 * 4];
Kore::u8 geometryConstants[1024 * 4];
Kore::u8 tessControlConstants[1024 * 4];
Kore::u8 tessEvalConstants[1024 * 4];

using namespace Kore;

namespace {
	static const int QUEUE_SLOT_COUNT = 3;
	int currentBackBuffer_ = 0;
	ID3D12CommandAllocator* commandAllocators_[QUEUE_SLOT_COUNT];
	ID3D12GraphicsCommandList* commandLists_[QUEUE_SLOT_COUNT];
	D3D12_VIEWPORT viewport_;
	D3D12_RECT rectScissor_;
	ID3D12Resource* renderTarget_;
	ID3D12DescriptorHeap* renderTargetDescriptorHeap_;
	ID3D12CommandQueue* commandQueue_;
	UINT64 currentFenceValue_;
	UINT64 fenceValues_[QUEUE_SLOT_COUNT];
	HANDLE frameFenceEvents_[QUEUE_SLOT_COUNT];
	ID3D12Fence* frameFences_[QUEUE_SLOT_COUNT];
	IDXGISwapChain* swapChain_;
	ID3D12Fence* uploadFence_;
	ID3D12GraphicsCommandList* initCommandList_;
	ID3D12CommandAllocator* initCommandAllocator_;
	ID3D12DescriptorHeap* srvDescriptorHeap_;
	ID3D12Resource* constantBuffers_[QUEUE_SLOT_COUNT];

	struct RenderEnvironment {
		ID3D12Device* device;
		ID3D12CommandQueue* queue;
		IDXGISwapChain* swapChain;
	};

	RenderEnvironment CreateDeviceAndSwapChainHelper(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL minimumFeatureLevel, const DXGI_SWAP_CHAIN_DESC* swapChainDesc) {
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

	void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void CreateRenderTargetView() {
		const D3D12_RESOURCE_DESC resourceDesc = renderTarget_->GetDesc();

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		device_->CreateRenderTargetView(renderTarget_, &viewDesc, renderTargetDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
	}

	void SetupSwapChain() {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&renderTargetDescriptorHeap_));

		currentFenceValue_ = 0;

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			frameFenceEvents_[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fenceValues_[i] = 0;
			device_->CreateFence(currentFenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFences_[i]));
		}

		swapChain_->GetBuffer(currentBackBuffer_, IID_PPV_ARGS(&renderTarget_));
		CreateRenderTargetView();
	}

	void CreateDeviceAndSwapChain(int width, int height, HWND window) {
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

		auto renderEnv = CreateDeviceAndSwapChainHelper(nullptr, D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

		device_ = renderEnv.device;
		commandQueue_ = renderEnv.queue;
		swapChain_ = renderEnv.swapChain;

		SetupSwapChain();
	}

	void CreateAllocatorsAndCommandLists() {
		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators_[i]));
			device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators_[i], nullptr, IID_PPV_ARGS(&commandLists_[i]));
			commandLists_[i]->Close();
		}
	}

	void CreateViewportScissor(int width, int height) {
		rectScissor_ = { 0, 0, width, height };
		viewport_ = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	}

	void CreateRootSignature() {
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = 1;

		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NodeMask = 0;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap_));

		ID3DBlob* rootBlob;
		ID3DBlob* errorBlob;

		CD3DX12_ROOT_PARAMETER parameters[2];

		CD3DX12_DESCRIPTOR_RANGE range{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
		parameters[0].InitAsDescriptorTable(1, &range);

		parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_STATIC_SAMPLER_DESC samplers[1];
		samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;

		descRootSignature.Init(2, parameters, 1, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);
		device_->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	}

	void CreateConstantBuffer() {
		struct ConstantBuffer {
			float x, y, z, w;
		};

		static const ConstantBuffer cb = { 0, 0, 0, 0 };

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffers_[i]));

			void* p;
			constantBuffers_[i]->Map(0, nullptr, &p);
			::memcpy(p, &cb, sizeof(cb));
			constantBuffers_[i]->Unmap(0, nullptr);
		}
	}

	void Initialize(int width, int height, HWND window) {
		//window_.reset(new Window("Anteru's D3D12 sample", 512, 512));

		CreateDeviceAndSwapChain(width, height, window);
		CreateAllocatorsAndCommandLists();
		CreateViewportScissor(width, height);
		CreateRootSignature();

		//CreatePipelineStateObject();
		CreateConstantBuffer();

		device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence_));

		device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&initCommandAllocator_));
		device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator_, nullptr, IID_PPV_ARGS(&initCommandList_));

		//CreateMeshBuffers();
		//CreateTexture();

		initCommandList_->Close();

		ID3D12CommandList* commandLists[] = { initCommandList_ };
		commandQueue_->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
		commandQueue_->Signal(uploadFence_, 1);

		HANDLE waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		WaitForFence(uploadFence_, 1, waitEvent);

		initCommandAllocator_->Reset();
		initCommandList_->Release(); // check me
		initCommandAllocator_->Release(); // check me

		CloseHandle(waitEvent);
	}

	void Shutdown() {
		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			WaitForFence(frameFences_[i], fenceValues_[i], frameFenceEvents_[i]);
		}

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			CloseHandle(frameFenceEvents_[i]);
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
	Initialize(renderTargetWidth, renderTargetHeight, hwnd);

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

	commandList->SetPipelineState(ProgramImpl::_current->pso_);
	commandList->SetGraphicsRootSignature(rootSignature_);

	ID3D12DescriptorHeap* heaps[] = { srvDescriptorHeap_ };
	commandList->SetDescriptorHeaps(1, heaps);

	commandList->SetGraphicsRootDescriptorTable(0, srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());

	commandList->SetGraphicsRootConstantBufferView(1, constantBuffers_[currentBackBuffer_]->GetGPUVirtualAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, (D3D12_VERTEX_BUFFER_VIEW*)&VertexBuffer::_current->vertexBufferView_);
	commandList->IASetIndexBuffer((D3D12_INDEX_BUFFER_VIEW*)&IndexBuffer::_current->indexBufferView_);
	commandList->DrawIndexedInstanced(count, 1, 0, 0, 0);
}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {
	
}

void Graphics::begin() {
#ifdef SYS_WINDOWSRT
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif

	WaitForFence(frameFences_[currentBackBuffer_], fenceValues_[currentBackBuffer_], frameFenceEvents_[currentBackBuffer_]);

	commandAllocators_[currentBackBuffer_]->Reset();

	commandList = commandLists_[currentBackBuffer_];
	commandList->Reset(commandAllocators_[currentBackBuffer_], nullptr);
	commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &rectScissor_);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget_;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	static const float clearColor[] = { 0.042f, 0.042f, 0.042f, 1 };

	commandList->ClearRenderTargetView(renderTargetDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;

	commandList = commandLists_[currentBackBuffer_];
}

void Graphics::end() {
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget_;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* commandList = commandLists_[currentBackBuffer_];
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	commandQueue_->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
}

bool Graphics::vsynced() {
	return vsync;
}

unsigned Graphics::refreshRate() {
	return hz;
}

void Graphics::swapBuffers() {
	swapChain_->Present(1, 0);

	currentBackBuffer_ = (currentBackBuffer_ + 1) % QUEUE_SLOT_COUNT;
	swapChain_->GetBuffer(currentBackBuffer_, IID_PPV_ARGS(&renderTarget_));

	CreateRenderTargetView();

	const UINT64 fenceValue = currentFenceValue_;
	commandQueue_->Signal(frameFences_[currentBackBuffer_], fenceValue);
	fenceValues_[currentBackBuffer_] = fenceValue;
	++currentFenceValue_;
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
				floats[x + y * 3] = value.get(y, x);
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
	//context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void Graphics::setRenderTarget(RenderTarget* target, int) {
	//context->OMSetRenderTargets(1, &target->renderTargetView, nullptr);
}
