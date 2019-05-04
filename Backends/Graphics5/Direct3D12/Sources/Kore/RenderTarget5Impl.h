#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Resource;
struct ID3D12DescriptorHeap;

struct D3D12Viewport {
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

struct D3D12Rect {
	long left;
	long top;
	long right;
	long bottom;
};

enum RenderTargetResourceState { RenderTargetResourceStateUndefined, RenderTargetResourceStateRenderTarget, RenderTargetResourceStateTexture };

typedef struct {
	ID3D12Resource *renderTarget;
	ID3D12DescriptorHeap *renderTargetDescriptorHeap;
	ID3D12DescriptorHeap *srvDescriptorHeap;
	ID3D12DescriptorHeap *depthStencilDescriptorHeap;
	ID3D12Resource *depthStencilTexture;
	D3D12Viewport viewport;
	D3D12Rect scissor;
	int stage;
	RenderTargetResourceState resourceState;
} RenderTarget5Impl;

#ifdef __cplusplus
}
#endif
