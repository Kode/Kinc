#ifndef KOPE_WEBGPU_PIPELINE_STRUCTS_HEADER
#define KOPE_WEBGPU_PIPELINE_STRUCTS_HEADER

#include <kope/graphics5/buffer.h>
#include <kope/graphics5/commandlist.h>
#include <kope/graphics5/device.h>
#include <kope/graphics5/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kope_webgpu_vertex_step_mode { KOPE_WEBGPU_VERTEX_STEP_MODE_VERTEX, KOPE_WEBGPU_VERTEX_STEP_MODE_INSTANCE } kope_webgpu_vertex_step_mode;

typedef enum kope_webgpu_vertex_format {
	KOPE_WEBGPU_VERTEX_FORMAT_UINT8X2,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT8X4,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT8X2,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT8X4,
	KOPE_WEBGPU_VERTEX_FORMAT_UNORM8X2,
	KOPE_WEBGPU_VERTEX_FORMAT_UNORM8X4,
	KOPE_WEBGPU_VERTEX_FORMAT_SNORM8X2,
	KOPE_WEBGPU_VERTEX_FORMAT_SNORM8X4,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT16X2,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT16X4,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT16X2,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT16X4,
	KOPE_WEBGPU_VERTEX_FORMAT_UNORM16X2,
	KOPE_WEBGPU_VERTEX_FORMAT_UNORM16X4,
	KOPE_WEBGPU_VERTEX_FORMAT_SNORM16X2,
	KOPE_WEBGPU_VERTEX_FORMAT_SNORM16X4,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT16X2,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT16X4,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT32,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT32X2,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT32X3,
	KOPE_WEBGPU_VERTEX_FORMAT_FLOAT32X4,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT32,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT32X2,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT32X3,
	KOPE_WEBGPU_VERTEX_FORMAT_UINT32X4,
	KOPE_WEBGPU_VERTEX_FORMAT_SIN32,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT32X2,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT32X3,
	KOPE_WEBGPU_VERTEX_FORMAT_SINT32X4,
	KOPE_WEBGPU_VERTEX_FORMAT_UNORM10_10_10_2
} kope_webgpu_vertex_format;

typedef struct kope_webgpu_vertex_attribute {
	kope_webgpu_vertex_format format;
	uint64_t offset;
	uint32_t shader_location;
} kope_webgpu_vertex_attribute;

#define KOPE_WEBGPU_MAX_VERTEX_ATTRIBUTES 32

typedef struct kope_webgpu_vertex_buffer_layout {
	uint64_t array_stride;
	kope_webgpu_vertex_step_mode step_mode;
	kope_webgpu_vertex_attribute attributes[KOPE_WEBGPU_MAX_VERTEX_ATTRIBUTES];
	size_t attributes_count;
} kope_webgpu_vertex_buffer_layout;

typedef struct kope_webgpu_shader {
	uint8_t *data;
	size_t size;
} kope_webgpu_shader;

#define KOPE_WEBGPU_MAX_VERTEX_BUFFERS 16

typedef struct kope_webgpu_vertex_state {
	kope_webgpu_shader shader;
	kope_webgpu_vertex_buffer_layout buffers[KOPE_WEBGPU_MAX_VERTEX_BUFFERS];
	size_t buffers_count;
} kope_webgpu_vertex_state;

typedef enum kope_webgpu_primitive_topology {
	KOPE_WEBGPU_PRIMITIVE_TOPOLOGY_POINT_LIST,
	KOPE_WEBGPU_PRIMITIVE_TOPOLOGY_LINE_LIST,
	KOPE_WEBGPU_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	KOPE_WEBGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	KOPE_WEBGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
} kope_webgpu_primitive_topology;

typedef enum kope_webgpu_front_face { KOPE_WEBGPU_FRONT_FACE_CCW, KOPE_WEBGPU_FRONT_FACE_CW } kope_webgpu_front_face;

typedef enum kope_webgpu_cull_mode { KOPE_WEBGPU_CULL_MODE_NONE, KOPE_WEBGPU_CULL_MODE_FRONT, KOPE_WEBGPU_CULL_MODE_BACK } kope_webgpu_cull_mode;

typedef struct kope_webgpu_primitive_state {
	kope_webgpu_primitive_topology topology;
	kope_g5_index_format strip_index_format;
	kope_webgpu_front_face front_face;
	kope_webgpu_cull_mode cull_mode;
	bool unclipped_depth;
} kope_webgpu_primitive_state;

typedef enum kope_webgpu_stencil_operation {
	KOPE_WEBGPU_STENCIL_OPERATION_KEEP,
	KOPE_WEBGPU_STENCIL_OPERATION_ZERO,
	KOPE_WEBGPU_STENCIL_OPERATION_REPLACE,
	KOPE_WEBGPU_STENCIL_OPERATION_INVERT,
	KOPE_WEBGPU_STENCIL_OPERATION_INCREMENT_CLAMP,
	KOPE_WEBGPU_STENCIL_OPERATION_DECREMENT_CLAMP,
	KOPE_WEBGPU_STENCIL_OPERATION_INCREMENT_WRAP,
	KOPE_WEBGPU_STENCIL_OPERATION_DECREMENT_WRAP
} kope_webgpu_stencil_operation;

typedef struct kope_webgpu_stencil_face_state {
	kope_g5_compare_function compare;
	kope_webgpu_stencil_operation fail_op;
	kope_webgpu_stencil_operation depth_fail_op;
	kope_webgpu_stencil_operation pass_op;
} kope_webgpu_stencil_face_state;

typedef struct kope_webgpu_depth_stencil_state {
	kope_g5_texture_format format;
	bool depth_write_enabled;
	kope_g5_compare_function depth_compare;
	kope_webgpu_stencil_face_state stencil_front;
	kope_webgpu_stencil_face_state stencil_back;
	uint32_t stencil_read_mask;
	uint32_t stencil_write_mask;
	int32_t depth_bias;
	float depth_bias_slope_scale;
	float depth_bias_clamp;
} kope_webgpu_depth_stencil_state;

typedef struct kope_webgpu_multisample_state {
	uint32_t count;
	uint32_t mask;
	bool alpha_to_coverage_enabled;
} kope_webgpu_multisample_state;

typedef enum kope_webgpu_blend_operation {
	KOPE_WEBGPU_BLEND_OPERATION_ADD,
	KOPE_WEBGPU_BLEND_OPERATION_SUBTRACT,
	KOPE_WEBGPU_BLEND_OPERATION_REVERSE_SUBTRACT,
	KOPE_WEBGPU_BLEND_OPERATION_MIN,
	KOPE_WEBGPU_BLEND_OPERATION_MAX
} kope_webgpu_blend_operation;

typedef enum kope_webgpu_blend_factor {
	KOPE_WEBGPU_BLEND_FACTOR_ZERO,
	KOPE_WEBGPU_BLEND_FACTOR_ONE,
	KOPE_WEBGPU_BLEND_FACTOR_SRC,
	KOPE_WEBGPU_BLEND_FACTOR_ONE_MINUS_SRC,
	KOPE_WEBGPU_BLEND_FACTOR_SRC_ALPHA,
	KOPE_WEBGPU_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	KOPE_WEBGPU_BLEND_FACTOR_DST,
	KOPE_WEBGPU_BLEND_FACTOR_ONE_MINUS_DST,
	KOPE_WEBGPU_BLEND_FACTOR_DST_ALPHA,
	KOPE_WEBGPU_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	KOPE_WEBGPU_BLEND_FACTOR_SRC_ALPHA_SATURATED,
	KOPE_WEBGPU_BLEND_FACTOR_CONSTANT,
	KOPE_WEBGPU_BLEND_FACTOR_ONE_MINUS_CONSTANT
} kope_webgpu_blend_factor;

typedef struct kope_webgpu_blend_component {
	kope_webgpu_blend_operation operation;
	kope_webgpu_blend_factor src_factor;
	kope_webgpu_blend_factor dst_factor;
} kope_webgpu_blend_component;

typedef struct kope_webgpu_blend_state {
	kope_webgpu_blend_component color;
	kope_webgpu_blend_component alpha;
} kope_webgpu_blend_state;

typedef enum kope_webgpu_color_write_flags {
	KOPE_WEBGPU_COLOR_WRITE_FLAGS_RED = 0x1,
	KOPE_WEBGPU_COLOR_WRITE_FLAGS_GREEN = 0x2,
	KOPE_WEBGPU_COLOR_WRITE_FLAGS_BLUE = 0x4,
	KOPE_WEBGPU_COLOR_WRITE_FLAGS_ALPHA = 0x8,
	KOPE_WEBGPU_COLOR_WRITE_FLAGS_ALL = 0xF
} kope_webgpu_color_write_flags;

typedef struct kope_webgpu_color_target_state {
	kope_g5_texture_format format;
	kope_webgpu_blend_state blend;
	uint32_t write_mask;
} kope_webgpu_color_target_state;

#define KOPE_WEBGPU_MAX_COLOR_TARGETS 8

typedef struct kope_webgpu_fragment_state {
	kope_webgpu_shader shader;
	kope_webgpu_color_target_state targets[KOPE_WEBGPU_MAX_COLOR_TARGETS];
	size_t targets_count;
} kope_webgpu_fragment_state;

typedef struct kope_webgpu_render_pipeline_parameters {
	kope_webgpu_vertex_state vertex;
	kope_webgpu_primitive_state primitive;
	kope_webgpu_depth_stencil_state depth_stencil;
	kope_webgpu_multisample_state multisample;
	kope_webgpu_fragment_state fragment;
} kope_webgpu_render_pipeline_parameters;

typedef struct kope_webgpu_render_pipeline {
	int nothing;
} kope_webgpu_render_pipeline;

typedef struct kope_webgpu_compute_pipeline_parameters {
	kope_webgpu_shader shader;
} kope_webgpu_compute_pipeline_parameters;

typedef struct kope_webgpu_compute_pipeline {
	int nothing;
} kope_webgpu_compute_pipeline;

typedef struct kope_webgpu_ray_pipeline_parameters {
	kope_webgpu_shader gen_shader;
	kope_webgpu_shader miss_shader;
	kope_webgpu_shader closest_shader;
	kope_webgpu_shader intersection_shader;
	kope_webgpu_shader any_shader;
} kope_webgpu_ray_pipeline_parameters;

typedef struct kope_webgpu_ray_pipeline {
	int nothing;
} kope_webgpu_ray_pipeline;

#ifdef __cplusplus
}
#endif

#endif
