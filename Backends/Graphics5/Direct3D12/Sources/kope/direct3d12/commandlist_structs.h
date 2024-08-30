#ifndef KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER
#define KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kope_d3d12_device;
struct kope_d3d12_texture;
struct ID3D12Fence;

#define KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT 3

typedef struct kope_d3d12_command_list {
	struct kope_d3d12_device *device;

	struct ID3D12CommandAllocator *allocator[KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT];
	uint32_t current_allocator_index;

	struct ID3D12GraphicsCommandList *list;
	kope_d3d12_texture *render_pass_framebuffer;

	uint64_t run_index;
	struct ID3D12Fence *fence;
	HANDLE event;
} kope_d3d12_command_list;

#ifdef __cplusplus
}
#endif

#endif
