#pragma once

#include <Kore/PipelineState5Impl.h>

#include <Kinc/Graphics5/VertexStructure.h>

#include "Graphics.h"
#include "ConstantLocation.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_shader;

typedef struct kinc_g5_pipeline {
	kinc_g5_vertex_structure_t *inputLayout[16];
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;
	struct kinc_g5_shader *geometryShader;
	struct kinc_g5_shader *tessellationControlShader;
	struct kinc_g5_shader *tessellationEvaluationShader;

	kinc_g5_cull_mode_t cullMode;

	bool depthWrite;
	kinc_g5_compare_mode_t depthMode;

	kinc_g5_compare_mode_t stencilMode;
	kinc_g5_stencil_action_t stencilBothPass;
	kinc_g5_stencil_action_t stencilDepthFail;
	kinc_g5_stencil_action_t stencilFail;
	int stencilReferenceValue;
	int stencilReadMask;
	int stencilWriteMask;

	// One, Zero deactivates blending
	kinc_g5_blending_operation_t blendSource;
	kinc_g5_blending_operation_t blendDestination;
	// BlendingOperation blendOperation;
	kinc_g5_blending_operation_t alphaBlendSource;
	kinc_g5_blending_operation_t alphaBlendDestination;
	// BlendingOperation alphaBlendOperation;

	bool colorWriteMaskRed[8]; // Per render target
	bool colorWriteMaskGreen[8];
	bool colorWriteMaskBlue[8];
	bool colorWriteMaskAlpha[8];

	bool conservativeRasterization;

	PipelineState5Impl impl;
} kinc_g5_pipeline_t;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_internal_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline);
kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name);
kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name);

#ifdef __cplusplus
}
#endif
