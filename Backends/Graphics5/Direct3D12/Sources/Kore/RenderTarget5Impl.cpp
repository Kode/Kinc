#include "pch.h"

#include "Direct3D12.h"
#include "RenderTarget5Impl.h"

#include <Kinc/Graphics5/RenderTarget.h>
#include <Kinc/Graphics5/Texture.h>
#include <Kore/Log.h>
#include <Kore/SystemMicrosoft.h>

#ifdef KORE_WINDOWS
#include <dxgi1_4.h>
#endif

static const int textureCount = 16;
extern kinc_g5_texture_t *currentTextures[textureCount];
extern kinc_g5_render_target_t *currentRenderTargets[textureCount];
extern IDXGISwapChain* swapChain;

namespace {
	ID3D12Fence* renderFence;

	void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent) {
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}

	void createRenderTargetView(ID3D12Resource* renderTarget, ID3D12DescriptorHeap* renderTargetDescriptorHeap, DXGI_FORMAT format) {
		const D3D12_RESOURCE_DESC resourceDesc = renderTarget->GetDesc();

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = format;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(renderTarget, &viewDesc, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

static DXGI_FORMAT convertFormat(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
	#ifdef KORE_WINDOWS
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	#else
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	#endif
	}
}

void kinc_g5_render_target_init(kinc_g5_render_target_t *render_target, int width, int height, int depthBufferBits, bool antialiasing,
                                kinc_g5_render_target_format_t format, int stencilBufferBits,
                                int contextId) {
	render_target->texWidth = render_target->width = width;
	render_target->texHeight = render_target->height = height;
	render_target->impl.stage = 0;
	render_target->impl.stage_depth = -1;
	render_target->impl.renderTargetReadback = nullptr;

	render_target->impl.resourceState = RenderTargetResourceStateUndefined;

	DXGI_FORMAT dxgiFormat = convertFormat(format);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = dxgiFormat;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	    &CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, render_target->texWidth, render_target->texHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
	                                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTarget));

	D3D12_RENDER_TARGET_VIEW_DESC view;
	const D3D12_RESOURCE_DESC resourceDesc = render_target->impl.renderTarget->GetDesc();
	view.Format = dxgiFormat;
	view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	view.Texture2D.MipSlice = 0;
	view.Texture2D.PlaneSlice = 0;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTargetDescriptorHeap));

	device->CreateRenderTargetView(render_target->impl.renderTarget, &view,
	                               render_target->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = dxgiFormat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(render_target->impl.renderTarget, &shaderResourceViewDesc,
	                                 render_target->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	if (depthBufferBits > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.depthStencilDescriptorHeap));

		CD3DX12_RESOURCE_DESC depthTexture(D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, width, height, 1, 1, DXGI_FORMAT_D32_FLOAT, 1, 0,
		                                   D3D12_TEXTURE_LAYOUT_UNKNOWN, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthTexture,
		                                D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_GRAPHICS_PPV_ARGS(&render_target->impl.depthStencilTexture));

		device->CreateDepthStencilView(render_target->impl.depthStencilTexture, nullptr,
		                               render_target->impl.depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		// Reading depth texture as a shader resource
		D3D12_DESCRIPTOR_HEAP_DESC srvDepthHeapDesc = {};
		srvDepthHeapDesc.NumDescriptors = 1;
		srvDepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDepthHeapDesc.NodeMask = 0;
		srvDepthHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&srvDepthHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.srvDepthDescriptorHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDepthViewDesc = {};
		srvDepthViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDepthViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDepthViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDepthViewDesc.Texture2D.MipLevels = 1;
		srvDepthViewDesc.Texture2D.MostDetailedMip = 0;
		srvDepthViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		device->CreateShaderResourceView(render_target->impl.depthStencilTexture, &srvDepthViewDesc,
		                                 render_target->impl.srvDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	else {
		render_target->impl.depthStencilDescriptorHeap = nullptr;
		render_target->impl.depthStencilTexture = nullptr;
		render_target->impl.srvDepthDescriptorHeap = nullptr;
	}

	render_target->impl.scissor = {0, 0, width, height};
	render_target->impl.viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};

	if (contextId < 0) { // encoded backbuffer index
		swapChain->GetBuffer(-contextId - 1, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTarget));
		createRenderTargetView(render_target->impl.renderTarget, render_target->impl.renderTargetDescriptorHeap, dxgiFormat);
	}
}

void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *render_target, int cubeMapSize, int depthBufferBits, bool antialiasing, kinc_g5_render_target_format_t format, int stencilBufferBits,
                                     int contextId) {
	render_target->impl.stage = 0;
	render_target->impl.stage_depth = -1;
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *render_target) {
	if (currentRenderTargets[render_target->impl.stage] == render_target) {
		currentRenderTargets[render_target->impl.stage] = NULL;
	}
	render_target->impl.renderTarget->Release();
	render_target->impl.renderTargetDescriptorHeap->Release();
	render_target->impl.srvDescriptorHeap->Release();
	if (render_target->impl.depthStencilTexture != NULL) {
		render_target->impl.depthStencilTexture->Release();
		render_target->impl.depthStencilDescriptorHeap->Release();
		render_target->impl.srvDepthDescriptorHeap->Release();
	}
}

void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *render_target, kinc_g5_texture_unit_t unit) {
	if (unit.impl.unit < 0) return;
	render_target->impl.stage = unit.impl.unit;
	currentRenderTargets[render_target->impl.stage] = render_target;
	currentTextures[render_target->impl.stage] = nullptr;
}

void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *render_target, kinc_g5_texture_unit_t unit) {
	if (unit.impl.unit < 0) return;
	render_target->impl.stage_depth = unit.impl.unit;
	currentRenderTargets[render_target->impl.stage_depth] = render_target;
	currentTextures[render_target->impl.stage_depth] = nullptr;
}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *render_target, kinc_g5_render_target_t *source) {
	render_target->impl.depthStencilDescriptorHeap = source->impl.depthStencilDescriptorHeap;
	render_target->impl.srvDepthDescriptorHeap = source->impl.srvDepthDescriptorHeap;
	render_target->impl.depthStencilTexture = source->impl.depthStencilTexture;
}
