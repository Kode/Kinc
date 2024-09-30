#ifndef KOPE_D3D12_BUFFER_STRUCTS_HEADER
#define KOPE_D3D12_BUFFER_STRUCTS_HEADER

#include "d3d12mini.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kope_g5_device;

typedef struct kope_d3d12_buffer {
	struct kope_g5_device *device;

	struct ID3D12Resource *resource;
	uint32_t resource_state;
	size_t size;

	bool cpu_accessible;
	uint64_t latest_execution_index;
} kope_d3d12_buffer;

#ifdef __cplusplus
}
#endif

#endif
