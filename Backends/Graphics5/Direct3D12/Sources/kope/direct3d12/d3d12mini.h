#ifndef KOPE_D3D12_MINI_HEADER
#define KOPE_D3D12_MINI_HEADER

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12Device;
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

#ifdef __cplusplus
}
#endif

#endif
