#ifndef KOPE_G5_FENCE_HEADER
#define KOPE_G5_FENCE_HEADER

#include <kope/global.h>

#include "api.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/fence_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_fence {
	KOPE_G5_IMPL(fence);
} kope_g5_fence;

KOPE_FUNC void kope_g5_fence_destroy(kope_g5_fence *fence);

#ifdef __cplusplus
}
#endif

#endif
