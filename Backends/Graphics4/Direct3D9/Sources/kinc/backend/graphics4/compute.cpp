#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/math/core.h>

void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *source, int length) {}

void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader) {}

kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_constant_location_t location = {0};
	return location;
}

kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_texture_unit_t unit = {0};
	return unit;
}

void kinc_g4_set_compute_shader(kinc_g4_compute_shader *shader) {}

void kinc_g4_compute(int x, int y, int z) {}
