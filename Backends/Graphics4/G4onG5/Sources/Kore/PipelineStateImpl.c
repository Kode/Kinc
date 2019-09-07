#include "pch.h"

#include "PipelineStateImpl.h"

#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics4/shader.h>

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *pipe) {
	kinc_g4_internal_pipeline_set_defaults(pipe);
	kinc_g5_pipeline_init(&pipe->impl._pipeline);
}

void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *pipe) {
	kinc_g5_pipeline_destroy(&pipe->impl._pipeline);
}

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *pipe, const char *name) {
	kinc_g4_constant_location_t location;
	location.impl._location = kinc_g5_pipeline_get_constant_location(&pipe->impl._pipeline, name);
	return location;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *pipe, const char *name) {
	kinc_g4_texture_unit_t unit;
	unit.impl._unit = kinc_g5_pipeline_get_texture_unit(&pipe->impl._pipeline, name);
	return unit;
}

void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *pipe) {
	for (int i = 0; i < 16; ++i) {
		pipe->impl._pipeline.inputLayout[i] = pipe->input_layout[i];
	}
	pipe->impl._pipeline.vertexShader = &pipe->vertex_shader->impl._shader;
	pipe->impl._pipeline.fragmentShader = &pipe->fragment_shader->impl._shader;
	pipe->impl._pipeline.geometryShader = pipe->geometry_shader != NULL ? &pipe->geometry_shader->impl._shader : NULL;
	pipe->impl._pipeline.tessellationControlShader = pipe->tessellation_control_shader != NULL ? &pipe->tessellation_control_shader->impl._shader : NULL;
	pipe->impl._pipeline.tessellationEvaluationShader = pipe->tessellation_evaluation_shader != NULL ? &pipe->tessellation_evaluation_shader->impl._shader : NULL;
	pipe->impl._pipeline.blendSource = (kinc_g5_blending_operation_t)pipe->blend_source;
	pipe->impl._pipeline.blendDestination = (kinc_g5_blending_operation_t)pipe->blend_destination;
	pipe->impl._pipeline.alphaBlendSource = (kinc_g5_blending_operation_t)pipe->alpha_blend_source;
	pipe->impl._pipeline.alphaBlendDestination = (kinc_g5_blending_operation_t)pipe->alpha_blend_destination;
	pipe->impl._pipeline.cullMode = (kinc_g5_cull_mode_t)pipe->cull_mode;
	pipe->impl._pipeline.depthMode = (kinc_g5_compare_mode_t)pipe->depth_mode;
	pipe->impl._pipeline.depthWrite = pipe->depth_write;
	for (int i = 0; i < 8; ++i) {
		pipe->impl._pipeline.colorWriteMaskRed[i] = pipe->color_write_mask_red[i];
		pipe->impl._pipeline.colorWriteMaskGreen[i] = pipe->color_write_mask_green[i];
		pipe->impl._pipeline.colorWriteMaskBlue[i] = pipe->color_write_mask_blue[i];
		pipe->impl._pipeline.colorWriteMaskAlpha[i] = pipe->color_write_mask_alpha[i];
	}
	kinc_g5_pipeline_compile(&pipe->impl._pipeline);
}
