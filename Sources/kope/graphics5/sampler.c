#include "sampler.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/sampler_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/sampler_functions.h>
#endif

void kope_g5_sampler_set_name(kope_g5_sampler *sampler, const char *name) {
	KOPE_G5_CALL2(sampler_set_name, sampler, name);
}
