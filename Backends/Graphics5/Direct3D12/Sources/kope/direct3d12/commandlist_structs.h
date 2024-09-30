#ifndef KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER
#define KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kope_d3d12_device;
struct kope_d3d12_texture;
struct kope_d3d12_compute_pipeline;
struct kope_d3d12_ray_pipeline;
struct kope_d3d12_rendery_pipeline;
struct kope_g5_query_set;

struct ID3D12Fence;

// Allocators can not be re-used while a command-list is executing. We carry along a bag of allocators so we only have to wait when we ran out of in-flight
// allocators. Increasing this value exchanges more memory against potentially less wait-times (depending on actual command-list usage).
#define KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT 3

#define KOPE_D3D12_COMMAND_LIST_MAX_QUEUED_BUFFER_ACCESSES 256

typedef struct kope_d3d12_command_list {
	struct kope_d3d12_device *device;

	struct ID3D12CommandAllocator *allocator[KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT];
	uint64_t allocator_execution_index[KOPE_D3D12_COMMAND_LIST_ALLOCATOR_COUNT];
	uint8_t current_allocator_index;

	struct ID3D12GraphicsCommandList4 *list;

	struct ID3D12DescriptorHeap *rtv_descriptors;
	uint32_t rtv_increment;

	struct ID3D12DescriptorHeap *dsv_descriptor;
	uint32_t dsv_increment;

	// set when a framebuffer is attached to a render-pass so we don't render into it during scan-out
	uint64_t blocking_frame_index;

	struct kope_d3d12_render_pipeline *render_pipe;

	struct kope_d3d12_compute_pipeline *compute_pipe;

	struct kope_d3d12_ray_pipeline *ray_pipe;

	struct kope_g5_query_set *occlusion_query_set;
	uint32_t current_occlusion_query_index;

	struct kope_g5_query_set *timestamp_query_set;
	uint32_t timestamp_beginning_of_pass_write_index;
	uint32_t timestamp_end_of_pass_write_index;

	kope_g5_buffer *queued_buffer_accesses[KOPE_D3D12_COMMAND_LIST_MAX_QUEUED_BUFFER_ACCESSES];
	uint32_t queued_buffer_accesses_count;

	bool presenting;
} kope_d3d12_command_list;

#ifdef __cplusplus
}
#endif

#endif
