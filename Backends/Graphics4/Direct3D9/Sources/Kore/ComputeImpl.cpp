#include "pch.h"

#include "ComputeImpl.h"

#include <Kinc/Compute/Compute.h>
#include <Kinc/Graphics4/RenderTarget.h>
#include <Kinc/Graphics4/Texture.h>
#include <Kinc/Math/Core.h>

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *source, int length) {}

void kinc_compute_shader_destroy(kinc_compute_shader_t *shader) {}

kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_constant_location_t location = {0};
	return location;
}

kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_texture_unit_t unit = {0};
	return unit;
}

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value) {}

void kinc_compute_set_int(kinc_compute_constant_location_t location, int value) {}

void kinc_compute_set_float(kinc_compute_constant_location_t location, float value) {}

void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2) {}

void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3) {}

void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4) {}

void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count) {}

void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, kinc_matrix4x4_t *value) {}

void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, kinc_matrix3x3_t *value) {}

void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture_t *texture, kinc_compute_access_t access) {}

void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *target, kinc_compute_access_t access) {}

void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture_t *texture) {}

void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *target) {}

void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target_t *target) {}

void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_shader(kinc_compute_shader_t *shader) {}

void kinc_compute(int x, int y, int z) {}
