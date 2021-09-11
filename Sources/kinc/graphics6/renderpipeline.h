#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics6/renderpipeline.h>

#include "texture.h"
#include "vertexlayout.h"
#include "bindgroup.h"

/*! \file renderpipeline.h
    \brief Provides functions for setting up render pipelines.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_primitive_topology {
	KINC_G6_PRIMITIVE_TOPOLOGY_POINT_LIST,
	KINC_G6_PRIMITIVE_TOPOLOGY_LINE_LIST,
	KINC_G6_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
} kinc_g6_primitive_topology_t;

typedef enum kinc_g6_enum_format { KINC_G6_INDEX_FORMAT_UINT16, KINC_G6_INDEX_FORMAT_UINT32 } kinc_g6_index_format_t;

typedef enum kinc_g6_front_face { KINC_G6_FRONT_FACE_CCW, KINC_G6_FRONT_FACE_CW } kinc_g6_front_face_t;

typedef enum kinc_g6_cull_mode { KINC_G6_CULL_MODE_NONE, KINC_G6_CULL_MODE_FRONT, KINC_G6_CULL_MODE_BACK } kinc_g6_cull_mode_t;

typedef enum kinc_g6_blend_factor {
	KINC_G6_BLEND_FACTOR_ZERO,
	KINC_G6_BLEND_FACTOR_ONE,
	KINC_G6_BLEND_FACTOR_SRC,
	KINC_G6_BLEND_FACTOR_ONE_MINUS_ALPHA,
	KINC_G6_BLEND_FACTOR_SRC_ALPHA,
	KINC_G6_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	KINC_G6_BLEND_FACTOR_DST,
	KINC_G6_BLEND_FACTOR_ONE_MINUS_DST,
	KINC_G6_BLEND_FACTOR_DST_ALPHA,
	KINC_G6_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	KINC_G6_BLEND_FACTOR_SRC_ALPHA_SATURATED,
	KINC_G6_BLEND_FACTOR_CONSTANT,
	KINC_G6_BLEND_FACTOR_ONE_MINUS_CONSTANT
} kinc_g6_blend_factor_t;

typedef enum kinc_g6_blend_operation {
	KINC_G6_BLEND_OPERATION_ADD,
	KINC_G6_BLEND_OPERATION_SUBSTRACT,
	KINC_G6_BLEND_OPERATION_REVERSE_SUBSTRACT,
	KINC_G6_BLEND_OPERATION_MIN,
	KINC_G6_BLEND_OPERATION_MAX
} kinc_g6_blend_operation_t;

typedef enum kinc_g6_compare_function {
	KINC_G6_COMPARE_FUNCTION_NEVER,
	KINC_G6_COMPARE_FUNCTION_LESS,
	KINC_G6_COMPARE_FUNCTION_EQUAL,
	KINC_G6_COMPARE_FUNCTION_LESS_EQUAL,
	KINC_G6_COMPARE_FUNCTION_GREATER,
	KINC_G6_COMPARE_FUNCTION_NOT_EQUAL,
	KINC_G6_COMPARE_FUNCTION_GREATER_EQUAL,
	KINC_G6_COMPARE_FUNCTION_ALWAYS
} kinc_g6_compare_function_t;

typedef enum king_g6_stencil_operation {
	KINC_G6_STENCIL_OPERATION_KEEP,
	KINC_G6_STENCIL_OPERATION_ZERO,
	KINC_G6_STENCIL_OPERATION_REPLACE,
	KINC_G6_STENCIL_OPERATION_INVERT,
	KINC_G6_STENCIL_OPERATION_INCREMENT_CLAMP,
	KINC_G6_STENCIL_OPERATION_DECREMENT_CLAMP,
	KINC_G6_STENCIL_OPERATION_INCREMENT_WRAP,
	KINC_G6_STENCIL_OPERATION_DECREMENT_WRAP
} kinc_g6_stencil_operation_t;

struct kinc_g6_pipeline_layout;

typedef struct kinc_g6_render_pipeline_descriptor {
	kinc_g6_vertex_layout_t *input_layout[16];
	struct kinc_g6_shader *vertex_shader;
	struct kinc_g6_shader *fragment_shader;

	// Primitive state
	struct {
		kinc_g6_primitive_topology_t topology;
		kinc_g6_index_format_t strip_index_format;
		kinc_g6_front_face_t front_face;
		kinc_g6_cull_mode_t cull_mode;
	};

	// Depth and Stencil state
	struct {
		bool enable_depth;
		bool enable_stencil;

		kinc_g6_texture_format_t depth_format;
		bool depth_write;
		kinc_g6_compare_function_t depth_compare;

		kinc_g6_compare_function_t stencil_compare_front;
		kinc_g6_stencil_operation_t stencil_fail_front;
		kinc_g6_stencil_operation_t depth_fail_front;
		kinc_g6_stencil_operation_t stencil_pass_front;

		kinc_g6_compare_function_t stencil_compare_back;
		kinc_g6_stencil_operation_t stencil_fail_back;
		kinc_g6_stencil_operation_t depth_fail_back;
		kinc_g6_stencil_operation_t stencil_pass_back;

		uint32_t stencil_read_mask;
		uint32_t stencil_write_mask;
		uint32_t stencil_reference;
		
		int32_t depth_bias;
		float depth_bias_slope_scale;
		float depth_bias_clamp;
	};

	// Multisample state
	struct {
		uint32_t sample_count;
		uint32_t sample_mask;
		bool sample_alpha_to_coverage_enabled;
	};

	// Color State

	int attachment_count;
	struct {
		kinc_g6_texture_format_t format;

		bool blend_enable;

		kinc_g6_blend_operation_t operation;
		kinc_g6_blend_factor_t blend_src;
		kinc_g6_blend_factor_t blend_dst;

		kinc_g6_blend_operation_t alpha_operation;
		kinc_g6_blend_factor_t alpha_blend_src;
		kinc_g6_blend_factor_t alpha_blend_dst;

		bool color_write_mask_red;
		bool color_write_mask_green;
		bool color_write_mask_blue;
		bool color_write_mask_alpha;
	} attachments[8];
	struct kinc_g6_pipeline_layout *layout;
} kinc_g6_render_pipeline_descriptor_t;

typedef struct kinc_g6_render_pipeline {
	kinc_g6_render_pipeline_impl_t impl;
} kinc_g6_render_pipeline_t;

typedef struct kinc_g6_pipeline_layout_descriptor {
	uint32_t count;
	struct kinc_g6_bind_group_layout **bind_groups;
} kinc_g6_pipeline_layout_descriptor_t;

typedef struct kinc_g6_pipeline_layout {
	kinc_g6_pipeline_layout_impl_t impl;
} kinc_g6_pipeline_layout_t;

KINC_FUNC void kinc_g6_render_pipeline_defaults(kinc_g6_render_pipeline_descriptor_t *pipeline);
KINC_FUNC void kinc_g6_render_pipeline_init(kinc_g6_render_pipeline_t *pipeline, const kinc_g6_render_pipeline_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_render_pipeline_destroy(kinc_g6_render_pipeline_t *pipeline);

KINC_FUNC void kinc_g6_pipeline_layout_init(kinc_g6_pipeline_layout_t *layout, const kinc_g6_pipeline_layout_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_pipeline_layout_destroy(kinc_g6_pipeline_layout_t *layout);

#ifdef __cplusplus
}
#endif