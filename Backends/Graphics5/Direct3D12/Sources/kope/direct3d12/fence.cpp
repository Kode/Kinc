#include "fence_functions.h"

#include <kope/graphics5/fence.h>

#include <kinc/backend/SystemMicrosoft.h>

void kope_d3d12_fence_destroy(kope_g5_fence *fence) {
	fence->d3d12.fence->Release();
}
