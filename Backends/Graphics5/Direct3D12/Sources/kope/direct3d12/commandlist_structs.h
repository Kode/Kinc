#ifndef KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER
#define KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kope_d3d12_device;
struct kope_d3d12_texture;
struct ID3D12Fence;

// Allocators can not be re-used while a command-list is executing. We carry along a bag of allocators so we only have to wait when we ran out of in-flight
// allocators. Increasing this value exchanges more memory against potentially less wait-times (depending on actual command-list usage).
#define KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT 3

typedef struct kope_d3d12_command_list {
	struct kope_d3d12_device *device;

	struct ID3D12CommandAllocator *allocator[KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT];

	struct ID3D12GraphicsCommandList *list;

	// a bunch of variables used to figure out what allocators can be reused
	uint64_t execution_index;
	struct ID3D12Fence *fence;
	HANDLE event;

	// set when a framebuffer is attached to a render-pass so we don't render into it during scan-out
	uint64_t blocking_frame_index;

	bool compute_pipeline_set;

	bool presenting;
} kope_d3d12_command_list;

#ifdef __cplusplus
}
#endif

#endif
