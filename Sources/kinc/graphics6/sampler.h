#pragma once

#include <kinc/global.h>

#include "renderpipeline.h"
#include <kinc/backend/graphics6/sampler.h>

/*! \file sampler.h
    \brief Provides functions for setting up and using samplers.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_address_mode {
	KINC_G6_ADDRESS_MODE_CLAMP_TO_EDGE,
	KINC_G6_ADDRESS_MODE_REPEAT,
	KINC_G6_ADDRESS_MODE_MIRROR_REPEAT
} kinc_g6_address_mode_t;

typedef enum kinc_g6_filter_mode { KINC_G6_FILTER_MODE_NEAREST, KINC_G6_FILTER_MODE_LINEAR } kinc_g6_filter_mode_t;

typedef struct kinc_g6_sampler_descriptor {
	kinc_g6_address_mode_t address_mode_u;
	kinc_g6_address_mode_t address_mode_v;
	kinc_g6_address_mode_t address_mode_w;

	kinc_g6_filter_mode_t mag_filter;
	kinc_g6_filter_mode_t min_filter;
	kinc_g6_filter_mode_t mipmap_filter;

	float lod_min_clamp;
	float lod_max_clamp;

	bool enable_compare;
	enum kinc_g6_compare_function compare;

	bool enable_anisotropy;
	uint16_t max_anisotropy;
} kinc_g6_sampler_descriptor_t;

typedef struct kinc_g6_sampler {
	kinc_g6_sampler_impl_t impl;
} kinc_g6_sampler_t;

KINC_FUNC void kinc_g6_internal_sampler_defaults(kinc_g6_sampler_descriptor_t *sampler);
KINC_FUNC void kinc_g6_sampler_init(kinc_g6_sampler_t *sampler, const kinc_g6_sampler_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_sampler_destroy(kinc_g6_sampler_t *sampler);

#ifdef __cplusplus
}
#endif