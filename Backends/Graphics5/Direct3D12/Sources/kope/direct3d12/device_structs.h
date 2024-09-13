#ifndef KOPE_D3D12_DEVICE_STRUCTS_HEADER
#define KOPE_D3D12_DEVICE_STRUCTS_HEADER

#include "d3d12mini.h"

#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/texture.h>
#include <kope/util/indexallocator.h>
#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12CommandAllocator;
struct ID3D12DescriptorHeap;

#define KOPE_D3D12_FRAME_COUNT 2

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

	uint32_t cbv_srv_uav_increment;
	uint32_t sampler_increment;

	uint32_t framebuffer_index;
	kope_g5_texture framebuffer_textures[KOPE_D3D12_FRAME_COUNT];

	struct ID3D12Fence *frame_fence;
	HANDLE frame_event;
	uint64_t current_frame_index;

	struct ID3D12DescriptorHeap *descriptor_heap;
	oa_allocator_t descriptor_heap_allocator;

	struct ID3D12DescriptorHeap *sampler_heap;
	oa_allocator_t sampler_heap_allocator;

	struct ID3D12DescriptorHeap *all_samplers;
	kope_index_allocator sampler_index_allocator;

	kope_g5_command_list management_list;
} kope_d3d12_device;

#ifdef __cplusplus
}
#endif

#endif
