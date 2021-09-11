#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/pipeline.h>

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipe) {}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_constant_location_t location;

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;

	return unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {}
