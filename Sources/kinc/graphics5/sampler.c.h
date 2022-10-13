#include "sampler.h"

void kinc_g5_sampler_descriptor_defaults(kinc_g5_sampler_descriptor_t *descriptor) {
	descriptor->u_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;
	descriptor->v_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;
	descriptor->w_addressing = KINC_G5_TEXTURE_ADDRESSING_CLAMP;

	descriptor->magnification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	descriptor->minification_filter = KINC_G5_TEXTURE_FILTER_POINT;
	descriptor->mipmap_filter = KINC_G5_MIPMAP_FILTER_POINT;

	descriptor->lod_min_clamp = 0;
	descriptor->lod_max_clamp = 32;

	descriptor->is_comparison = false;
	descriptor->compare_mode = KINC_G5_COMPARE_MODE_ALWAYS;

	descriptor->max_anisotropy = 1;
}
