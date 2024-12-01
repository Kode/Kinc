#include "fence.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/fence_functions.h>
#endif

void kope_g5_fence_destroy(kope_g5_fence *fence) {
	KOPE_G5_CALL1(fence_destroy, fence);
}
