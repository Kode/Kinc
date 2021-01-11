#pragma once

#include <Kore/ComputeImpl.h>

#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/textureunit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_compute_shader_impl {
	int a;
} kinc_g5_compute_shader_ímpl_t;

typedef struct kinc_g5_compute_shader {
	kinc_g5_compute_shader_ímpl_t impl;
} kinc_g5_compute_shader_t;

KINC_FUNC void kinc_g5_compute_shader_init(kinc_g5_compute_shader_t *shader, void *source, int length);
KINC_FUNC void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader_t *shader);
KINC_FUNC kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader_t *shader, const char *name);
KINC_FUNC kinc_g5_texture_unit_t kinc_compute_g5_shader_get_texture_unit(kinc_g5_compute_shader_t *shader, const char *name);

#ifdef __cplusplus
}
#endif
