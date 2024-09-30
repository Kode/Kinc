#ifndef KOPE_G5_SAMPLER_HEADER
#define KOPE_G5_SAMPLER_HEADER

#include <kope/global.h>

#include "api.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/sampler_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/sampler_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_sampler {
	KOPE_G5_IMPL(sampler);
} kope_g5_sampler;

KOPE_FUNC void kope_g5_sampler_set_name(kope_g5_sampler *sampler, const char *name);

#ifdef __cplusplus
}
#endif

#endif
