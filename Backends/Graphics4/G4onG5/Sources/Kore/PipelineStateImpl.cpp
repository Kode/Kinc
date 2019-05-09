#include "pch.h"

#include "PipelineStateImpl.h"
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics5/PipelineState.h>

#include <Kore/Graphics4/Shader.h>

using namespace Kore;

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *pipe) {
	kinc_g5_pipeline_init(&pipe->impl._pipeline);
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
	pipe->impl._pipeline.geometryShader = pipe->geometry_shader != nullptr ? &pipe->geometry_shader->impl._shader : nullptr;
	pipe->impl._pipeline.tessellationControlShader = pipe->tessellation_control_shader != nullptr ? &pipe->tessellation_control_shader->impl._shader : nullptr;
	pipe->impl._pipeline.tessellationEvaluationShader = pipe->tessellation_evaluation_shader != nullptr ? &pipe->tessellation_evaluation_shader->impl._shader : nullptr;
	pipe->impl._pipeline.blendSource = (kinc_g5_blending_operation_t)pipe->blend_source;
	pipe->impl._pipeline.blendDestination = (kinc_g5_blending_operation_t)pipe->blend_destination;
	pipe->impl._pipeline.alphaBlendSource = (kinc_g5_blending_operation_t)pipe->alpha_blend_source;
	pipe->impl._pipeline.alphaBlendDestination = (kinc_g5_blending_operation_t)pipe->alpha_blend_destination;
	pipe->impl._pipeline.depthMode = (kinc_g5_compare_mode_t)pipe->depth_mode;
	pipe->impl._pipeline.depthWrite = pipe->depth_write;
	kinc_g5_pipeline_compile(&pipe->impl._pipeline);
}
