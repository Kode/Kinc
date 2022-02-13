#include "indexbuffer.h"
#include "pipeline.h"
#include "vertexbuffer.h"
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/math/core.h>
#include <kinc/window.h>
#ifdef KORE_WINDOWS
#include <dxgi1_4.h>
#undef CreateWindow
#endif
#include <kinc/system.h>
#ifdef KORE_WINDOWS
#include <kinc/backend/Windows.h>
#endif
#include <kinc/backend/SystemMicrosoft.h>

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

// ID3D12GraphicsCommandList* commandList;
ID3D12Resource *depthStencilTexture;
ID3D12CommandQueue *commandQueue;
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
ID3D12Resource *swapChainRenderTargets[QUEUE_SLOT_COUNT];
#else
IDXGISwapChain *swapChain;
#endif

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

#ifndef KORE_WINDOWS
#define DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC1
#define IDXGISwapChain IDXGISwapChain1
#endif

struct RenderEnvironment {
	ID3D12Device *device;
	ID3D12CommandQueue *queue;
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
	ID3D12Resource *renderTargets[QUEUE_SLOT_COUNT];
#else
	IDXGISwapChain *swapChain;
#endif
};

#ifndef KORE_WINDOWS
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
void createSwapChain(RenderEnvironment *env, int bufferCount);
#else
void createSwapChain(RenderEnvironment *env, const DXGI_SWAP_CHAIN_DESC1 *desc);
#endif
#endif

void createSamplersAndHeaps();
extern bool bilinearFiltering;

static D3D12_VIEWPORT viewport;
static D3D12_RECT rectScissor;
// ID3D12Resource* renderTarget;
// ID3D12DescriptorHeap* renderTargetDescriptorHeap;
static ID3D12DescriptorHeap *depthStencilDescriptorHeap;
static UINT64 currentFenceValue;
static UINT64 fenceValues[QUEUE_SLOT_COUNT];
static HANDLE frameFenceEvents[QUEUE_SLOT_COUNT];
static ID3D12Fence *frameFences[QUEUE_SLOT_COUNT];
static ID3D12Fence *uploadFence;
static ID3D12GraphicsCommandList *initCommandList;
static ID3D12CommandAllocator *initCommandAllocator;

static struct RenderEnvironment createDeviceAndSwapChainHelper(IDXGIAdapter *adapter, D3D_FEATURE_LEVEL minimumFeatureLevel,
                                                               const DXGI_SWAP_CHAIN_DESC *swapChainDesc) {
	struct RenderEnvironment result = {0};
#ifdef KORE_WINDOWS
	kinc_microsoft_affirm(D3D12CreateDevice((IUnknown *)adapter, minimumFeatureLevel, &IID_ID3D12Device, &result.device));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {0};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	kinc_microsoft_affirm(result.device->lpVtbl->CreateCommandQueue(result.device, &queueDesc, &IID_ID3D12CommandQueue, &result.queue));

	IDXGIFactory4 *dxgiFactory;
	kinc_microsoft_affirm(CreateDXGIFactory1(&IID_IDXGIFactory4, &dxgiFactory));

	DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
	kinc_microsoft_affirm(dxgiFactory->lpVtbl->CreateSwapChain(dxgiFactory, (IUnknown *)result.queue, &swapChainDescCopy, &result.swapChain));
#else
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
	createSwapChain(&result, QUEUE_SLOT_COUNT);
#else
	createSwapChain(&result, swapChainDesc);
#endif
#endif
	return result;
}

static void waitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
	if (fence->lpVtbl->GetCompletedValue(fence) < completionValue) {
		kinc_microsoft_affirm(fence->lpVtbl->SetEventOnCompletion(fence, completionValue, waitEvent));
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

static void setupSwapChain() {
	/*D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&renderTargetDescriptorHeap));*/

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {0};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	kinc_microsoft_affirm(device->lpVtbl->CreateDescriptorHeap(device, &dsvHeapDesc, &IID_ID3D12DescriptorHeap, &depthStencilDescriptorHeap));

	D3D12_RESOURCE_DESC depthTexture = {0};
	depthTexture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthTexture.Alignment = 0;
	depthTexture.Width = renderTargetWidth;
	depthTexture.Height = renderTargetHeight;
	depthTexture.DepthOrArraySize = 1;
	depthTexture.MipLevels = 1;
	depthTexture.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexture.SampleDesc.Count = 1;
	depthTexture.SampleDesc.Quality = 0;
	depthTexture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthTexture.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

	D3D12_CLEAR_VALUE clearValue = {0};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = {0};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &depthTexture,
	                                                              D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, &IID_ID3D12Resource, &depthStencilTexture));

	device->lpVtbl->CreateDepthStencilView(device, depthStencilTexture, NULL, GetCPUDescriptorHandle(depthStencilDescriptorHeap));

	currentFenceValue = 0;

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		frameFenceEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		fenceValues[i] = 0;
		device->lpVtbl->CreateFence(device, currentFenceValue, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &frameFences[i]);
	}

	//**swapChain->GetBuffer(currentBackBuffer, IID_GRAPHICS_PPV_ARGS(&renderTarget));
	//**createRenderTargetView();
}

static void createDeviceAndSwapChain(int width, int height, HWND window) {
#ifdef _DEBUG
	ID3D12Debug *debugController = NULL;
	D3D12GetDebugInterface(&IID_ID3D12Debug, &debugController);
	debugController->lpVtbl->EnableDebugLayer(debugController);
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

	struct RenderEnvironment renderEnv = createDeviceAndSwapChainHelper(NULL, D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

	device = renderEnv.device;
	commandQueue = renderEnv.queue;
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		swapChainRenderTargets[i] = renderEnv.renderTargets[i];
	}
#else
	swapChain = renderEnv.swapChain;
#endif

	setupSwapChain();
}

static void createViewportScissor(int width, int height) {
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = width;
	rectScissor.bottom = height;

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

static void createRootSignature() {
	ID3DBlob *rootBlob;
	ID3DBlob *errorBlob;

	D3D12_ROOT_PARAMETER parameters[4] = {0};

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = (UINT)textureCount;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;

	D3D12_DESCRIPTOR_RANGE range2;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	range2.NumDescriptors = (UINT)textureCount;
	range2.BaseShaderRegister = 0;
	range2.RegisterSpace = 0;
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	parameters[1].DescriptorTable.pDescriptorRanges = &range2;

	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameters[2].Descriptor.ShaderRegister = 0;
	parameters[2].Descriptor.RegisterSpace = 0;

	parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	parameters[3].Descriptor.ShaderRegister = 0;
	parameters[3].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC samplers[textureCount * 2];
	for (int i = 0; i < textureCount; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}
	for (int i = textureCount; i < textureCount * 2; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}

	D3D12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.NumParameters = 4;
	descRootSignature.pParameters = parameters;
	descRootSignature.NumStaticSamplers = 0;
	descRootSignature.pStaticSamplers = NULL;
	descRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	kinc_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
	device->lpVtbl->CreateRootSignature(device, 0, rootBlob->lpVtbl->GetBufferPointer(rootBlob), rootBlob->lpVtbl->GetBufferSize(rootBlob),
	                                    &IID_ID3D12RootSignature, &globalRootSignature);

	createSamplersAndHeaps();
}

static void createComputeRootSignature() {
	ID3DBlob *rootBlob;
	ID3DBlob *errorBlob;

	D3D12_ROOT_PARAMETER parameters[3] = {0};

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = (UINT)textureCount;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;

	D3D12_DESCRIPTOR_RANGE range2;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	range2.NumDescriptors = (UINT)textureCount;
	range2.BaseShaderRegister = 0;
	range2.RegisterSpace = 0;
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	parameters[1].DescriptorTable.pDescriptorRanges = &range2;

	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[2].Descriptor.ShaderRegister = 0;
	parameters[2].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC samplers[textureCount * 2];
	for (int i = 0; i < textureCount; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}
	for (int i = textureCount; i < textureCount * 2; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}

	D3D12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.NumParameters = 3;
	descRootSignature.pParameters = parameters;
	descRootSignature.NumStaticSamplers = 0;
	descRootSignature.pStaticSamplers = NULL;
	descRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	kinc_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
	device->lpVtbl->CreateRootSignature(device, 0, rootBlob->lpVtbl->GetBufferPointer(rootBlob), rootBlob->lpVtbl->GetBufferSize(rootBlob),
	                                    &IID_ID3D12RootSignature, &globalComputeRootSignature);

	// createSamplersAndHeaps();
}

static void initialize(int width, int height, HWND window) {
	createDeviceAndSwapChain(width, height, window);
	createViewportScissor(width, height);
	createRootSignature();
	createComputeRootSignature();

	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &uploadFence);

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &initCommandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, NULL, &IID_ID3D12CommandList, &initCommandList);

	initCommandList->lpVtbl->Close(initCommandList);

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)initCommandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);
	commandQueue->lpVtbl->Signal(commandQueue, uploadFence, 1);

	HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	waitForFence(uploadFence, 1, waitEvent);

	initCommandAllocator->lpVtbl->Reset(initCommandAllocator);
	initCommandList->lpVtbl->Release(initCommandList);           // check me
	initCommandAllocator->lpVtbl->Release(initCommandAllocator); // check me

	CloseHandle(waitEvent);
}

static void shutdown() {
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		waitForFence(frameFences[i], fenceValues[i], frameFenceEvents[i]);
	}

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		CloseHandle(frameFenceEvents[i]);
	}
}

static unsigned hz;
static bool vsync;

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

void kinc_g5_destroy_window(int window) {}

void kinc_g5_destroy() {}

void kinc_g5_init() {}

void kinc_g5_init_window(int window, int depthBufferBits, int stencilBufferBits, bool verticalSync) {
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
// void kinc_g5_draw_indexed_vertices_instanced(int instanceCount) {}

// void kinc_g5_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {}

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, kinc_g5_texture_direction_t dir, kinc_g5_texture_addressing_t addressing) {}

int kinc_g5_max_bound_textures(void) {
	return D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
}

// (DK) fancy macro's to generate a clickable warning message in visual studio, can be removed when setColorMask() is implemented
#define Stringize(L) #L
#define MakeString(M, L) M(L)
#define $Line MakeString(Stringize, __LINE__)
#define Warning __FILE__ "(" $Line ") : warning: "

static bool began = false;

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window) {
	if (began) return;
	began = true;

	currentBackBuffer = (currentBackBuffer + 1) % QUEUE_SLOT_COUNT;

	if (newRenderTargetWidth != renderTargetWidth || newRenderTargetHeight != renderTargetHeight) {
		depthStencilDescriptorHeap->lpVtbl->Release(depthStencilDescriptorHeap);
		depthStencilTexture->lpVtbl->Release(depthStencilTexture);
#ifndef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
		kinc_microsoft_affirm(swapChain->lpVtbl->ResizeBuffers(swapChain, 2, newRenderTargetWidth, newRenderTargetHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
#endif
		setupSwapChain();
		renderTargetWidth = newRenderTargetWidth;
		renderTargetHeight = newRenderTargetHeight;
		currentBackBuffer = 0;
	}

	const UINT64 fenceValue = currentFenceValue;
	commandQueue->lpVtbl->Signal(commandQueue, frameFences[currentBackBuffer], fenceValue);
	fenceValues[currentBackBuffer] = fenceValue;
	++currentFenceValue;

#ifdef KORE_WINDOWSAPP
	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
#endif

	waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

	// static const float clearColor[] = {0.042f, 0.042f, 0.042f, 1};

	// commandList->ClearRenderTargetView(GetCPUDescriptorHandle(renderTargetDescriptorHeap), clearColor, 0, nullptr);

	// commandList->ClearDepthStencilView(GetCPUDescriptorHandle(depthStencilDescriptorHeap), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;
}

void kinc_g5_end(int window) {
	began = false;
}

bool kinc_g5_vsynced() {
	return true;
}

bool kinc_window_vsynced(int window) {
	return true;
}

void kinc_internal_resize(int window, int width, int height) {
	if (width == 0 || height == 0) return;
	newRenderTargetWidth = width;
	newRenderTargetHeight = height;
}

void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {}

#ifndef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
bool kinc_g5_swap_buffers() {
	kinc_microsoft_affirm(swapChain->lpVtbl->Present(swapChain, vsync, 0));
	return true;
}
#endif

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
