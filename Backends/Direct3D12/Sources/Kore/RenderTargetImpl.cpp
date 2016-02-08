#include "pch.h"
#include "Direct3D12.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>
#include <Kore/Log.h>
#include "d3dx12.h"

using namespace Kore;

static const int textureCount = 16;
extern Texture* currentTextures[textureCount];
extern RenderTarget* currentRenderTargets[textureCount];

namespace {
	ID3D12Fence* renderFence;

	void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}
}

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texWidth, texHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&renderTarget));

#if defined(_DEBUG)
	log(Info, "depthBufferBits not implemented yet, using target defaults");
	log(Info, "stencilBufferBits not implemented yet, using target defaults");
#endif

	D3D12_RENDER_TARGET_VIEW_DESC view;
	const D3D12_RESOURCE_DESC resourceDesc = renderTarget->GetDesc();
	view.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	view.Texture2D.MipSlice = 0;
	view.Texture2D.PlaneSlice = 0;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&renderTargetDescriptorHeap));

	device->CreateRenderTargetView(renderTarget, &view, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(renderTarget, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	scissor = { 0, 0, width, height };
	viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
}

extern void graphicsFlushAndWait();

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	graphicsFlushAndWait();
	this->stage = unit.unit;
	currentRenderTargets[stage] = this;
	currentTextures[stage] = nullptr;
}
