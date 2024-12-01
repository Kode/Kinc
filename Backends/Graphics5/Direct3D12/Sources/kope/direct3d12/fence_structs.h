#ifndef KOPE_D3D12_FENCE_STRUCTS_HEADER
#define KOPE_D3D12_FENCE_STRUCTS_HEADER

#include <kope/graphics5/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Fence;

typedef struct kope_d3d12_fence {
	struct ID3D12Fence *fence;
} kope_d3d12_fence;

#ifdef __cplusplus
}
#endif

#endif
