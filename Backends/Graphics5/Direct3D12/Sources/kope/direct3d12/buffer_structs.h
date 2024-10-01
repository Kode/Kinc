#ifndef KOPE_D3D12_BUFFER_STRUCTS_HEADER
#define KOPE_D3D12_BUFFER_STRUCTS_HEADER

#include "d3d12mini.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_g5_device;

#define KOPE_D3D12_MAX_BUFFER_RANGES 16

typedef struct kope_d3d12_buffer_range {
	uint64_t offset;
	uint64_t size;
	uint64_t execution_index;
} kope_d3d12_buffer_range;

typedef struct kope_d3d12_buffer {
	struct kope_g5_device *device;

	struct ID3D12Resource *resource;
	uint32_t resource_state;
	size_t size;

	void *locked_data;
	uint64_t locked_data_offset;
	uint64_t locked_data_size;

	bool cpu_write;
	bool cpu_read;

	kope_d3d12_buffer_range ranges[KOPE_D3D12_MAX_BUFFER_RANGES];
	uint32_t ranges_count;
} kope_d3d12_buffer;

#ifdef __cplusplus
}
#endif

#endif
