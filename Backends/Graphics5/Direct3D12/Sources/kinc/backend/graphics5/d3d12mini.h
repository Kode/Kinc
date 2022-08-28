#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
typedef void *HANDLE;
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

#ifdef __cplusplus
}
#endif
