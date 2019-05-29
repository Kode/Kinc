#pragma once

#include <Kinc/Math/Matrix.h>

#include "ConstantLocation.h"
#include "TextureUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g4_pipeline;
struct kinc_g4_render_target;
struct kinc_g4_texture;
struct kinc_g4_texture_array;

typedef enum {
	KINC_G4_TEXTURE_ADDRESSING_REPEAT,
	KINC_G4_TEXTURE_ADDRESSING_MIRROR,
	KINC_G4_TEXTURE_ADDRESSING_CLAMP,
	KINC_G4_TEXTURE_ADDRESSING_BORDER
} Kinc_G4_TextureAddressing;

typedef enum {
	KINC_G4_TEXTURE_DIRECTION_U,
	KINC_G4_TEXTURE_DIRECTION_V,
	KINC_G4_TEXTURE_DIRECTION_W
} Kinc_G4_TextureDirection;

typedef enum {
	KINC_G4_TEXTURE_OPERATION_MODULATE,
	KINC_G4_TEXTURE_OPERATION_SELECT_FIRST,
	KINC_G4_TEXTURE_OPERATION_SELECT_SECOND
} Kinc_G4_TextureOperation;

typedef enum {
	KINC_G4_TEXTURE_ARGUMENT_CURRENT_COLOR,
	KINC_G4_TEXTURE_ARGUMENT_TEXTURE_COLOR
} Kinc_G4_TextureArgument;

typedef enum {
	KINC_G4_TEXTURE_FILTER_POINT,
	KINC_G4_TEXTURE_FILTER_LINEAR,
	KINC_G4_TEXTURE_FILTER_ANISOTROPIC
} Kinc_G4_TextureFilter;

typedef enum {
	KINC_G4_MIPMAP_FILTER_NONE,
	KINC_G4_MIPMAP_FILTER_POINT,
	KINC_G4_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} Kinc_G4_MipmapFilter;

void kinc_g4_init(int window, int depthBufferBits, int stencilBufferBits, bool vSync);

void kinc_g4_destroy(int window);

void kinc_g4_flush();

void kinc_g4_begin(int window);

void kinc_g4_end(int window);

bool kinc_g4_swap_buffers();

#define KINC_G4_CLEAR_COLOR   1
#define KINC_G4_CLEAR_DEPTH   2
#define KINC_G4_CLEAR_STENCIL 4

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil);

void kinc_g4_viewport(int x, int y, int width, int height);

void kinc_g4_scissor(int x, int y, int width, int height);

void kinc_g4_disable_scissor();

void kinc_g4_draw_indexed_vertices();

void kinc_g4_draw_indexed_vertices_from_to(int start, int count);

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount);

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count);

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);

void kinc_g4_set_pipeline(struct kinc_g4_pipeline *pipeline);

void kinc_g4_set_stencil_reference_value(int value);

void kinc_g4_set_texture_operation(Kinc_G4_TextureOperation operation, Kinc_G4_TextureArgument arg1, Kinc_G4_TextureArgument arg2);

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value);
void kinc_g4_set_float(kinc_g4_constant_location_t location, float value);
void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2);
void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3);
void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4);
void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count);
void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value);
void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, Kinc_Matrix3x3 *value);
void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, Kinc_Matrix4x4 *value);

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, Kinc_G4_TextureFilter filter);

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, Kinc_G4_TextureFilter filter);

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, Kinc_G4_TextureFilter filter);

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, Kinc_G4_TextureFilter filter);

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, Kinc_G4_MipmapFilter filter);

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, Kinc_G4_MipmapFilter filter);

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled);

bool kinc_g4_render_targets_inverted_y();

bool kinc_g4_non_pow2_textures_supported();

void kinc_g4_restore_render_target();

void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count);

void kinc_g4_set_render_target_face(struct kinc_g4_render_target *texture, int face);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture);

bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery);

void kinc_g4_delete_occlusion_query(unsigned occlusionQuery);

void kinc_g4_start_occlusion_query(unsigned occlusionQuery);

void kinc_g4_end_occlusion_query(unsigned occlusionQuery);

bool kinc_g4_are_query_results_available(unsigned occlusionQuery);

void kinc_g4_get_query_results(unsigned occlusionQuery, unsigned *pixelCount);

void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, struct _Kinc_G4_TextureArray *array);

int kinc_g4_antialiasing_samples();

void kinc_g4_set_antialiasing_samples(int samples);

#ifdef __cplusplus
}
#endif
