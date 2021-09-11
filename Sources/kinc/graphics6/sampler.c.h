#include "sampler.h"

void kinc_g6_internal_sampler_defaults(kinc_g6_sampler_descriptor_t *sampler) {
	sampler->address_mode_u = KINC_G6_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler->address_mode_v = KINC_G6_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler->address_mode_w = KINC_G6_ADDRESS_MODE_CLAMP_TO_EDGE;

	sampler->mag_filter = KINC_G6_FILTER_MODE_NEAREST;
	sampler->min_filter = KINC_G6_FILTER_MODE_NEAREST;
	sampler->mipmap_filter = KINC_G6_FILTER_MODE_NEAREST;

	sampler->lod_min_clamp = 0;
	sampler->lod_max_clamp = 32;

	sampler->enable_compare = false;
	sampler->compare = KINC_G6_COMPARE_FUNCTION_NEVER;

	sampler->enable_anisotropy = false;
	sampler->max_anisotropy = 1;
}