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
	struct ID3D12Resource *renderTarget;
	struct ID3D12Resource *renderTargetReadback;
	struct ID3D12DescriptorHeap *renderTargetDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;
	struct ID3D12DescriptorHeap *depthStencilDescriptorHeap;
	struct ID3D12DescriptorHeap *srvDepthDescriptorHeap;
	struct ID3D12Resource *depthStencilTexture;
	struct D3D12Viewport viewport;
	struct D3D12Rect scissor;
	int stage;
	int stage_depth;
	enum RenderTargetResourceState resourceState;
} RenderTarget5Impl;

#ifdef __cplusplus
}
#endif
