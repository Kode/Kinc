#include "pch.h"

#include "Direct3D12.h"
#include "RenderTarget5Impl.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Log.h>
#include <Kore/WinError.h>

#ifdef KORE_WINDOWS
#include <dxgi1_4.h>
#endif

using namespace Kore;

static const int textureCount = 16;
extern Graphics5::Texture* currentTextures[textureCount];
extern Graphics5::RenderTarget* currentRenderTargets[textureCount];
extern IDXGISwapChain* swapChain;

namespace {
	ID3D12Fence* renderFence;

	void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void createRenderTargetView(ID3D12Resource* renderTarget, ID3D12DescriptorHeap* renderTargetDescriptorHeap) {
		const D3D12_RESOURCE_DESC resourceDesc = renderTarget->GetDesc();

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
#ifdef KORE_WINDOWS
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
#else
		viewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
#endif
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(renderTarget, &viewDesc, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

Graphics5::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

	resourceState = RenderTargetResourceStateUndefined;

#ifdef KORE_WINDOWS
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
#else
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
#endif

	device->CreateCommittedResource(
	    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	    &CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, texWidth, texHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
	    D3D12_RESOURCE_STATE_COMMON, nullptr, IID_GRAPHICS_PPV_ARGS(&renderTarget));

	D3D12_RENDER_TARGET_VIEW_DESC view;
	const D3D12_RESOURCE_DESC resourceDesc = renderTarget->GetDesc();
	view.Format = dxgiFormat;
	view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	view.Texture2D.MipSlice = 0;
	view.Texture2D.PlaneSlice = 0;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&renderTargetDescriptorHeap));

	device->CreateRenderTargetView(renderTarget, &view, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = dxgiFormat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(renderTarget, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	if (depthBufferBits > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(&depthStencilDescriptorHeap));

		CD3DX12_RESOURCE_DESC depthTexture(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0,
			width, height, 1, 1,
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
	}
	else {
		depthStencilDescriptorHeap = nullptr;
		depthStencilTexture = nullptr;
	}

	scissor = {0, 0, width, height};
	viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};

	if (contextId < 0) { // encoded backbuffer index
		swapChain->GetBuffer(-contextId - 1, IID_GRAPHICS_PPV_ARGS(&renderTarget));
		createRenderTargetView(renderTarget, renderTargetDescriptorHeap);
	}
}

Graphics5::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	
}

Graphics5::RenderTarget::~RenderTarget() {
	renderTarget->Release();
	renderTargetDescriptorHeap->Release();
	srvDescriptorHeap->Release();
}

namespace {
	void graphicsFlushAndWait() {
		/*commandList->Close();

		ID3D12CommandList* commandLists[] = {commandList};
		commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);

		const UINT64 fenceValue = currentFenceValue;
		commandQueue->Signal(frameFences[currentBackBuffer], fenceValue);
		fenceValues[currentBackBuffer] = fenceValue;
		++currentFenceValue;

		waitForFence(frameFences[currentBackBuffer], fenceValues[currentBackBuffer], frameFenceEvents[currentBackBuffer]);

		commandList->Reset(commandAllocators[currentBackBuffer], nullptr);
		commandList->OMSetRenderTargets(1, &renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), true, nullptr);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &rectScissor);*/
	}
}

void Graphics5::RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	graphicsFlushAndWait();
	this->stage = unit.unit;
	currentRenderTargets[stage] = this;
	currentTextures[stage] = nullptr;
}

void Graphics5::RenderTarget::useDepthAsTexture(TextureUnit unit) {

}

void Graphics5::RenderTarget::setDepthStencilFrom(RenderTarget* source) {

}
