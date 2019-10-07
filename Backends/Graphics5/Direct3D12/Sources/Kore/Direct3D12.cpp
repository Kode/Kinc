#include "pch.h"

#include "Direct3D12.h"
#include "IndexBuffer5Impl.h"
#include "PipelineState5Impl.h"
#include "VertexBuffer5Impl.h"
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/math/core.h>
#include <kinc/window.h>
#ifdef KORE_WINDOWS
#include <dxgi1_4.h>
#undef CreateWindow
#endif
#include <Kinc/System.h>
#ifdef KORE_WINDOWS
#include <Kore/Windows.h>
#endif
#include <Kore/SystemMicrosoft.h>
#include <wrl.h>

#include <type_traits>

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

int currentBackBuffer = -1;
ID3D12Device* device;
ID3D12RootSignature* rootSignature;
// ID3D12GraphicsCommandList* commandList;
ID3D12Resource* depthStencilTexture;
ID3D12CommandQueue* commandQueue;
IDXGISwapChain* swapChain;

extern "C" {
	int renderTargetWidth;
	int renderTargetHeight;
	int newRenderTargetWidth;
	int newRenderTargetHeight;
}

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
void createSwapChain(RenderEnvironment* env, const DXGI_SWAP_CHAIN_DESC1* desc);
#endif

void createSamplersAndHeaps();
extern bool bilinearFiltering;

namespace {
	D3D12_VIEWPORT viewport;
	D3D12_RECT rectScissor;
	// ID3D12Resource* renderTarget;
	// ID3D12DescriptorHeap* renderTargetDescriptorHeap;
	ID3D12DescriptorHeap* depthStencilDescriptorHeap;
	UINT64 currentFenceValue;
	UINT64 fenceValues[QUEUE_SLOT_COUNT];
	HANDLE frameFenceEvents[QUEUE_SLOT_COUNT];
	ID3D12Fence* frameFences[QUEUE_SLOT_COUNT];
	ID3D12Fence* uploadFence;
	ID3D12GraphicsCommandList* initCommandList;
	ID3D12CommandAllocator* initCommandAllocator;

	RenderEnvironment createDeviceAndSwapChainHelper(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL minimumFeatureLevel, const DXGI_SWAP_CHAIN_DESC* swapChainDesc) {
		RenderEnvironment result;
#ifdef KORE_WINDOWS
		kinc_microsoft_affirm(D3D12CreateDevice(adapter, minimumFeatureLevel, IID_PPV_ARGS(&result.device)));

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		kinc_microsoft_affirm(result.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&result.queue)));

		IDXGIFactory4* dxgiFactory;
		kinc_microsoft_affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

		DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
		kinc_microsoft_affirm(dxgiFactory->CreateSwapChain(result.queue, &swapChainDescCopy, &result.swapChain));
#else
		createSwapChain(&result, swapChainDesc);
#endif
		return result;
	}

	void waitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			kinc_microsoft_affirm(fence->SetEventOnCompletion(completionValue, waitEvent));
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void setupSwapChain() {
		/*D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&renderTargetDescriptorHeap));*/

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(&depthStencilDescriptorHeap));

		CD3DX12_RESOURCE_DESC depthTexture(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, renderTargetWidth, renderTargetHeight, 1, 1, DXGI_FORMAT_D32_FLOAT, 1, 0,
		                                   D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthTexture,
		                                D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_GRAPHICS_PPV_ARGS(&depthStencilTexture));

		device->CreateDepthStencilView(depthStencilTexture, nullptr, depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		currentFenceValue = 0;

		for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
			frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fenceValues[i] = 0;
			device->CreateFence(currentFenceValue, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&frameFences[i]));
		}

		//**swapChain->GetBuffer(currentBackBuffer, IID_GRAPHICS_PPV_ARGS(&renderTarget));
		//**createRenderTargetView();
	}

	void createDeviceAndSwapChain(int width, int height, HWND window) {
#ifdef _DEBUG
		ID3D12Debug* debugController = nullptr;
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

		CD3DX12_ROOT_PARAMETER parameters[4];

		CD3DX12_DESCRIPTOR_RANGE range{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)textureCount, 0};
		parameters[0].InitAsDescriptorTable(1, &range);
		CD3DX12_DESCRIPTOR_RANGE range2{D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, (UINT)textureCount, 0};
		parameters[1].InitAsDescriptorTable(1, &range2);

		parameters[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		parameters[3].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC samplers[textureCount * 2];
		for (int i = 0; i < textureCount; ++i) {
			samplers[i].Init(i, D3D12_FILTER_MIN_MAG_MIP_POINT);
		}
		for (int i = textureCount; i < textureCount * 2; ++i) {
			samplers[i].Init(i, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
		}

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(4, parameters, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		kinc_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
		device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(&rootSignature));

		createSamplersAndHeaps();
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
	    Microsoft::affirm(commandQueue->Signal(fence, fenceValues[currentFrame]));
	    Microsoft::affirm(fence->SetEventOnCompletion(fenceValues[currentFrame], fenceEvent));
	    WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
	    fenceValues[currentFrame]++;
	}*/
}

void kinc_g5_destroy(int window) {}

void kinc_g5_init(int window, int depthBufferBits, int stencilBufferBits, bool verticalSync) {
#ifdef KORE_WINDOWS
	HWND hwnd = kinc_windows_window_handle(window);
#else
	HWND hwnd = nullptr;
#endif
	vsync = verticalSync;
	newRenderTargetWidth = renderTargetWidth = kinc_width();
	newRenderTargetHeight = renderTargetHeight = kinc_height();
	initialize(renderTargetWidth, renderTargetHeight, hwnd);
}

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
void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

// (DK) fancy macro's to generate a clickable warning message in visual studio, can be removed when setColorMask() is implemented
#define Stringize(L) #L
#define MakeString(M, L) M(L)
#define $Line MakeString(Stringize, __LINE__)
#define Warning __FILE__ "(" $Line ") : warning: "

namespace {
	bool began = false;
}

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
	if (began) return;
	began = true;

	currentBackBuffer = (currentBackBuffer + 1) % QUEUE_SLOT_COUNT;

	if (newRenderTargetWidth != renderTargetWidth || newRenderTargetHeight != renderTargetHeight) {
		depthStencilDescriptorHeap->Release();
		depthStencilTexture->Release();
		kinc_microsoft_affirm(swapChain->ResizeBuffers(2, newRenderTargetWidth, newRenderTargetHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
		setupSwapChain();
		renderTargetWidth = newRenderTargetWidth;
		renderTargetHeight = newRenderTargetHeight;
		currentBackBuffer = 0;
	}

	const UINT64 fenceValue = currentFenceValue;
	commandQueue->Signal(frameFences[currentBackBuffer], fenceValue);
	fenceValues[currentBackBuffer] = fenceValue;
	++currentFenceValue;

#ifdef KORE_WINDOWSAPP
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif

	waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

	// static const float clearColor[] = {0.042f, 0.042f, 0.042f, 1};

	// commandList->ClearRenderTargetView(renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

	// commandList->ClearDepthStencilView(depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;
}

void kinc_g5_end(int window) {
	began = false;
}

bool kinc_g5_vsynced() {
	return true;
}

extern "C" bool kinc_window_vsynced(int window) {
	return true;
}

extern "C" void kinc_internal_resize(int window, int width, int height) {
	if (width == 0 || height == 0) return;
	newRenderTargetWidth = width;
	newRenderTargetHeight = height;
}

extern "C" void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {}

bool kinc_g5_swap_buffers() {
	kinc_microsoft_affirm(swapChain->Present(vsync, 0));
	return true;
}

void kinc_g5_flush() {}

void kinc_g5_set_texture_operation(kinc_g5_texture_operation_t operation, kinc_g5_texture_argument_t arg1, kinc_g5_texture_argument_t arg2) {}

void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	bilinearFiltering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, kinc_g5_texture_filter_t filter) {
	bilinearFiltering = filter != KINC_G5_TEXTURE_FILTER_POINT;
}

void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, kinc_g5_mipmap_filter_t filter) {}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}

bool kinc_g5_non_pow2_textures_supported() {
	return true;
}

void kinc_g5_set_render_target_face(kinc_g5_render_target_t *texture, int face) {}
/*
void Graphics5::setVertexBuffers(VertexBuffer** buffers, int count) {
    buffers[0]->_set(0);
}

void Graphics5::setIndexBuffer(IndexBuffer& buffer) {
    buffer._set();
}
*/
void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	kinc_g5_internal_texture_set(texture, unit.impl.unit);
}

bool kinc_g5_init_occlusion_query(unsigned *occlusionQuery) {
	return false;
}

void kinc_g5_set_image_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {}

void kinc_g5_delete_occlusion_query(unsigned occlusionQuery) {}

void kinc_g5_render_occlusion_query(unsigned occlusionQuery, int triangles) {}

bool kinc_g5_are_query_results_available(unsigned occlusionQuery) {
	return false;
}

void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount) {}

/*void Graphics5::setPipeline(PipelineState* pipeline) {
    pipeline->set(pipeline);
}*/
