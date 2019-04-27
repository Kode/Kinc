#pragma once

#include <Kore/Graphics1/Image.h>

#include "RenderTarget.h"
#include "Shader.h"
#include "Texture.h"
#include "TextureUnit.h"
#include "VertexStructure.h"

#include <Kore/Graphics5Impl.h>

#include <Kinc/Math/Matrix.h>
#include <Kinc/Math/Vector.h>

enum TextureAddressing { Repeat, Mirror, Clamp, Border };

enum TextureFilter { PointFilter, LinearFilter, AnisotropicFilter };

enum MipmapFilter {
	NoMipFilter,
	PointMipFilter,
	LinearMipFilter // linear texture filter + linear mip filter -> trilinear filter
};

enum RenderState {
	BlendingState,
	DepthTest,
	DepthTestCompare,
	/*Lighting,*/ DepthWrite,
	Normalize,
	BackfaceCulling,
	/*FogState, FogStartState, FogEndState, FogTypeState, FogColorState,*/ ScissorTestState,
	AlphaTestState,
	AlphaReferenceState
};

enum BlendingOperation {
	BlendOne,
	BlendZero,
	SourceAlpha,
	DestinationAlpha,
	InverseSourceAlpha,
	InverseDestinationAlpha,
	SourceColor,
	DestinationColor,
	InverseSourceColor,
	InverseDestinationColor
};

enum ZCompareMode {
	ZCompareAlways,
	ZCompareNever,
	ZCompareEqual,
	ZCompareNotEqual,
	ZCompareLess,
	ZCompareLessEqual,
	ZCompareGreater,
	ZCompareGreaterEqual
};

enum CullMode { Clockwise, CounterClockwise, NoCulling };

enum TexDir { U, V, W };

enum FogType { LinearFog };

enum RenderTargetFormat { Target32Bit, Target64BitFloat, Target32BitRedFloat, Target128BitFloat, Target16BitDepth, Target8BitRed };

enum StencilAction { Keep, Zero, Replace, Increment, IncrementWrap, Decrement, DecrementWrap, Invert };

enum TextureOperation { ModulateOperation, SelectFirstOperation, SelectSecondOperation };

enum TextureArgument { CurrentColorArgument, TextureColorArgument };
		
void kinc_g5_set_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture);
void kinc_g5_set_image_texture(kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture);

void kinc_g5_draw_indexed_vertices_instanced(int instanceCount);
void kinc_g5_draw_indexed_vertices_instanced(int instanceCount, int start, int count);

int kinc_g5_antialiasing_samples();
void kinc_g5_set_antialiasing_samples(int samples);

bool kinc_g5_render_targets_inverted_y();
void kinc_g5_set_render_target_face(kinc_g4_render_target_t *texture, int face);

void kinc_g5_begin(kinc_g4_render_target_t *renderTarget, int window);
void kinc_g5_end(int window = 0);
bool kinc_g5_swap_buffers();

void kinc_internal_g5_resize(int window, int width, int height);

void kinc_g5_set_texture_addressing(kinc_g5_texture_unit_t unit, TexDir dir, TextureAddressing addressing);
void kinc_g5_set_texture_magnification_filter(kinc_g5_texture_unit_t texunit, TextureFilter filter);
void kinc_g5_set_texture_minification_filter(kinc_g5_texture_unit_t texunit, TextureFilter filter);
void kinc_g5_set_texture_mipmap_filter(kinc_g5_texture_unit_t texunit, MipmapFilter filter);
void kinc_g5_set_texture_operation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2);

bool kinc_g5_non_pow2_textures_qupported();

// Occlusion Query
bool kinc_g5_init_occlusion_query(unsigned *occlusionQuery);
void kinc_g5_delete_occlusion_query(unsigned occlusionQuery);
void kinc_g5_render_occlusion_query(unsigned occlusionQuery, int triangles);
bool kinc_g5_are_query_results_available(unsigned occlusionQuery);
void kinc_g5_get_query_result(unsigned occlusionQuery, unsigned *pixelCount);

const unsigned ClearColorFlag = 1;
const unsigned ClearDepthFlag = 2;
const unsigned ClearStencilFlag = 4;

void kinc_g5_init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync = true);
void kinc_g5_destroy(int windowId);

extern bool kinc_g5_fullscreen;

void kinc_g5_flush();
