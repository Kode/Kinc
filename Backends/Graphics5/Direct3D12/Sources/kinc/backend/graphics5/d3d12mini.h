#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct IDXGISwapChain;

typedef void *HANDLE;
typedef unsigned __int64 UINT64;

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

#define KINC_INTERNAL_D3D12_SWAP_CHAIN_COUNT 2

struct dx_window {
#ifndef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
	struct IDXGISwapChain *swapChain;
#endif
	UINT64 current_fence_value;
	UINT64 fence_values[KINC_INTERNAL_D3D12_SWAP_CHAIN_COUNT];
	HANDLE frame_fence_events[KINC_INTERNAL_D3D12_SWAP_CHAIN_COUNT];
	struct ID3D12Fence *frame_fences[KINC_INTERNAL_D3D12_SWAP_CHAIN_COUNT];
	int width;
	int height;
	int new_width;
	int new_height;
	int current_backbuffer;
	bool vsync;
	int window_index;
};

struct dx_window *kinc_dx_current_window();

#ifdef __cplusplus
}
#endif
