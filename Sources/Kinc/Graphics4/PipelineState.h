#pragma once

#include <Kinc/Graphics4/ConstantLocation.h>
#include <Kinc/Graphics4/TextureUnit.h>

#include <Kore/PipelineStateImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _Kinc_G4_VertexStructure;
struct _Kinc_G4_Shader;

typedef enum {
	KINC_G4_BLEND_ONE,
	KINC_G4_BLEND_ZERO,
	KINC_G4_BLEND_SOURCE_ALPHA,
	KINC_G4_BLEND_DEST_ALPHA,
	KINC_G4_BLEND_INV_SOURCE_ALPHA,
	KINC_G4_BLEND_INV_DEST_ALPHA,
	KINC_G4_BLEND_SOURCE_COLOR,
	KINC_G4_BLEND_DEST_COLOR,
	KINC_G4_BLEND_INV_SOURCE_COLOR,
	KINC_G4_BLEND_INV_DEST_COLOR
} Kinc_G4_BlendingOperation;

typedef enum {
	KINC_G4_COMPARE_ALWAYS,
	KINC_G4_COMPARE_NEVER,
	KINC_G4_COMPARE_EQUAL,
	KINC_G4_COMPARE_NOT_EQUAL,
	KINC_G4_COMPARE_LESS,
	KINC_G4_COMPARE_LESS_EQUAL,
	KINC_G4_COMPARE_GREATER,
	KINC_G4_COMPARE_GREATER_EQUAL
} Kinc_G4_CompareMode;

typedef enum {
	KINC_G4_CULL_CLOCKWISE,
	KINC_G4_CULL_COUNTER_CLOCKWISE,
	KINC_G4_CULL_NOTHING
} Kinc_G4_CullMode;

typedef enum {
	KINC_G4_STENCIL_KEEP,
	KINC_G4_STENCIL_ZERO,
	KINC_G4_STENCIL_REPLACE,
	KINC_G4_STENCIL_INCREMENT,
	KINC_G4_STENCIL_INCREMENT_WRAP,
	KINC_G4_STENCIL_DECREMENT,
	KINC_G4_STENCIL_DECREMENT_WRAP,
	KINC_G4_STENCIL_INVERT
} Kinc_G4_StencilAction;

typedef struct _Kinc_G4_PipelineState {
	_Kinc_G4_VertexStructure *inputLayout[16];
	_Kinc_G4_Shader *vertexShader;
	_Kinc_G4_Shader *fragmentShader;
	_Kinc_G4_Shader *geometryShader;
	_Kinc_G4_Shader *tessellationControlShader;
	_Kinc_G4_Shader *tessellationEvaluationShader;

	Kinc_G4_CullMode cullMode;

	bool depthWrite;
	Kinc_G4_CompareMode depthMode;

	Kinc_G4_CompareMode stencilMode;
	Kinc_G4_StencilAction stencilBothPass;
	Kinc_G4_StencilAction stencilDepthFail;
	Kinc_G4_StencilAction stencilFail;
	int stencilReferenceValue;
	int stencilReadMask;
	int stencilWriteMask;

	// One, Zero deactivates blending
	Kinc_G4_BlendingOperation blendSource;
	Kinc_G4_BlendingOperation blendDestination;
	// BlendingOperation blendOperation;
	Kinc_G4_BlendingOperation alphaBlendSource;
	Kinc_G4_BlendingOperation alphaBlendDestination;
	// BlendingOperation alphaBlendOperation;

	bool colorWriteMaskRed[8]; // Per render target
	bool colorWriteMaskGreen[8];
	bool colorWriteMaskBlue[8];
	bool colorWriteMaskAlpha[8];

	bool conservativeRasterization;

	Kinc_G4_PipelineStateImpl impl;
} Kinc_G4_PipelineState;

void Kinc_G4_PipelineState_Create(Kinc_G4_PipelineState *state);
void Kinc_G4_PipelineState_Destroy(Kinc_G4_PipelineState *state);
void Kinc_G4_PipelineState_Compile(Kinc_G4_PipelineState *state);
Kinc_G4_ConstantLocation Kinc_G4_PipelineState_GetConstantLocation(Kinc_G4_PipelineState *state, const char *name);
Kinc_G4_TextureUnit Kinc_G4_PipelineState_GetTextureUnit(Kinc_G4_PipelineState *state, const char *name);

#ifdef __cplusplus
}
#endif
