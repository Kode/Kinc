#ifndef KOPE_D3D12_DESCRIPTORSET_STRUCTS_HEADER
#define KOPE_D3D12_DESCRIPTORSET_STRUCTS_HEADER

#include "d3d12mini.h"

#include "device_structs.h"

#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_descriptor_set {
	oa_allocation_t descriptor_allocation;
	size_t descriptor_count;

	size_t dynamic_descriptor_count;

	oa_allocation_t sampler_allocation;
	size_t sampler_count;
} kope_d3d12_descriptor_set;

#ifdef __cplusplus
}
#endif

#endif
