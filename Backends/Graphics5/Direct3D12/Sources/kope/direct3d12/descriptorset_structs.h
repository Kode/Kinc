#ifndef KOPE_D3D12_DESCRIPTORSET_STRUCTS_HEADER
#define KOPE_D3D12_DESCRIPTORSET_STRUCTS_HEADER

#include "d3d12mini.h"

#include <kope/util/offalloc/offalloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_descriptor_set {
	oa_allocation_t allocation;
	size_t descriptor_count;
} kope_d3d12_descriptor_set;

#ifdef __cplusplus
}
#endif

#endif
