#ifndef KOPE_VULKAN_FENCE_FUNCTIONS_HEADER
#define KOPE_VULKAN_FENCE_FUNCTIONS_HEADER

#include <kope/graphics5/fence.h>

#ifdef __cplusplus
extern "C" {
#endif

void kope_vulkan_fence_destroy(kope_g5_fence *fence);

#ifdef __cplusplus
}
#endif

#endif
