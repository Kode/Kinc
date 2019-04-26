#pragma once

#include <Kore/PipelineState5Impl.h>

#include <Kore/Graphics5/VertexStructure.h>

#include "Graphics.h"

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

	CullMode cullMode;

	bool depthWrite;
	ZCompareMode depthMode;

	ZCompareMode stencilMode;
	StencilAction stencilBothPass;
	StencilAction stencilDepthFail;
	StencilAction stencilFail;
	int stencilReferenceValue;
	int stencilReadMask;
	int stencilWriteMask;

	// One, Zero deactivates blending
	BlendingOperation blendSource;
	BlendingOperation blendDestination;
	// BlendingOperation blendOperation;
	BlendingOperation alphaBlendSource;
	BlendingOperation alphaBlendDestination;
	// BlendingOperation alphaBlendOperation;

	bool colorWriteMaskRed[8]; // Per render target
	bool colorWriteMaskGreen[8];
	bool colorWriteMaskBlue[8];
	bool colorWriteMaskAlpha[8];

	bool conservativeRasterization;
} kinc_g5_pipeline_t;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline);
void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline);
kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name);
kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name);

#ifdef __cplusplus
}
#endif
