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
	kinc_g4_texture_unit_t g4_unit = kinc_g4_pipeline_get_texture_unit(&pipe->impl.pipe, name);

	assert(KINC_G4_SHADER_TYPE_COUNT == KINC_G5_SHADER_TYPE_COUNT);
	kinc_g5_texture_unit_t g5_unit;
	memcpy(&g5_unit.stages[0], &g4_unit.stages[0], KINC_G5_SHADER_TYPE_COUNT * sizeof(int));

	return g5_unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	for (int i = 0; i < 16; ++i) {
		pipe->impl.pipe.input_layout[i] = pipe->inputLayout[i];
	}
	pipe->impl.pipe.vertex_shader = &pipe->vertexShader->impl.shader;
	pipe->impl.pipe.fragment_shader = &pipe->fragmentShader->impl.shader;
	kinc_g4_pipeline_compile(&pipe->impl.pipe);
}
