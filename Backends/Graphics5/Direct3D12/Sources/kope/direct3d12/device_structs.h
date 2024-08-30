#ifndef KOPE_D3D12_DEVICE_STRUCTS_HEADER
#define KOPE_D3D12_DEVICE_STRUCTS_HEADER

#include "d3d12mini.h"

#include <kope/util/indexallocator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_device {
	struct ID3D12Device *device;
	struct ID3D12CommandQueue *queue;
	struct IDXGISwapChain *swap_chain;

	struct ID3D12DescriptorHeap *all_rtvs;
	kope_index_allocator rtv_index_allocator;
	uint32_t rtv_increment;

	struct ID3D12DescriptorHeap *all_dsvs;
	kope_index_allocator dsv_index_allocator;
	uint32_t dsv_increment;

	uint32_t framebuffer_index;
	kope_g5_texture framebuffer_textures[2];

	struct ID3D12Fence *frame_fence;
	HANDLE frame_event;
	uint64_t current_frame_index;

} kope_d3d12_device;

#ifdef __cplusplus
}
#endif

#endif
