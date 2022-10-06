#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/pipeline.h>

#include <string.h>

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	kinc_g4_pipeline_init(&pipe->impl.pipe);
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipe) {
	kinc_g4_pipeline_destroy(&pipe->impl.pipe);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.location = kinc_g4_pipeline_get_constant_location(&pipe->impl.pipe, name);
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	unit.impl.unit = kinc_g4_pipeline_get_texture_unit(&pipe->impl.pipe, name);
	return unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	for (int i = 0; i < 16; ++i) {
		pipe->impl.pipe.input_layout[i] = pipe->inputLayout[i];
	}
	pipe->impl.pipe.vertex_shader = &pipe->vertexShader->impl.shader;
	pipe->impl.pipe.fragment_shader = &pipe->fragmentShader->impl.shader;
	kinc_g4_pipeline_compile(&pipe->impl.pipe);
}

bool kinc_g5_texture_unit_equals(kinc_g5_texture_unit_t *unit1, kinc_g5_texture_unit_t *unit2) {
	return memcmp(unit1, unit2, sizeof(kinc_g5_texture_unit_t)) == 0;
}
