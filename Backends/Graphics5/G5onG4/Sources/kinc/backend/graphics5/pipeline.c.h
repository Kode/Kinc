#include "kinc/graphics4/constantlocation.h"
#include "kinc/log.h"
#include "kinc/memory.h"
#include <kinc/graphics5/constantlocation.h>
#include <kinc/graphics5/pipeline.h>

#include <kinc/system.h>

void kinc_g4_pipeline_get_constant_locations(kinc_g4_pipeline_t *state, kinc_g4_constant_location_t *vertex_locations,
                                             kinc_g4_constant_location_t *fragment_locations, int *vertex_sizes, int *fragment_sizes, int *max_vertex,
                                             int *max_fragment);

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	kinc_g4_pipeline_init(&pipeline->impl.pipe);
	// int vertex_count = 0;
	// int fragment_count = 0;
	// kinc_g4_pipeline_get_constant_locations(&pipeline->impl.pipe, NULL, NULL, NULL, NULL, &vertex_count, &fragment_count);
	// pipeline->impl.vertex_locations = kinc_allocate(vertex_count * sizeof(kinc_g4_constant_location_t));
	// pipeline->impl.fragment_locations = kinc_allocate(fragment_count * sizeof(kinc_g4_constant_location_t));
	// pipeline->impl.vertex_sizes = kinc_allocate(vertex_count * sizeof(int));
	// pipeline->impl.fragment_sizes = kinc_allocate(fragment_count * sizeof(int));
	// if (pipeline->impl.vertex_locations == NULL || pipeline->impl.fragment_locations == NULL || pipeline->impl.vertex_sizes == NULL ||
	//     pipeline->impl.fragment_sizes == NULL) {
	// 	kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to allocate pipeline reflection data.");
	// 	kinc_stop();
	// }
	// else {
	// 	pipeline->impl.vertex_location_count = vertex_count;
	// 	pipeline->impl.fragment_location_count = fragment_count;
	// 	kinc_g4_pipeline_get_constant_locations(&pipeline->impl.pipe, pipeline->impl.vertex_locations, pipeline->impl.fragment_locations, pipeline->impl.vertex_sizes, pipeline->impl.fragment_sizes, &vertex_count,
	// 	                                        &fragment_count);
	// }
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
