#include "renderpipeline.h"

void kinc_g6_render_pipeline_defaults(kinc_g6_render_pipeline_descriptor_t *pipeline) {
	for (int i = 0; i < 16; ++i) {
		pipeline->input_layout[i] = NULL;
	}

	pipeline->vertex_shader = NULL;
	pipeline->fragment_shader = NULL;

	pipeline->topology = KINC_G6_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline->strip_index_format = KINC_G6_INDEX_FORMAT_UINT16;
	pipeline->front_face = KINC_G6_FRONT_FACE_CCW;
	pipeline->cull_mode = KINC_G6_CULL_MODE_NONE;

	pipeline->enable_depth = false;
	pipeline->enable_stencil = false;
	
	pipeline->depth_format = KINC_G6_TEXTURE_FORMAT_NONE;
	pipeline->depth_write = false;
	pipeline->depth_compare = KINC_G6_COMPARE_FUNCTION_ALWAYS;

	pipeline->stencil_compare_front = KINC_G6_COMPARE_FUNCTION_ALWAYS;
	pipeline->stencil_fail_front = KINC_G6_STENCIL_OPERATION_KEEP;
	pipeline->depth_fail_front = KINC_G6_STENCIL_OPERATION_KEEP;
	pipeline->stencil_pass_front = KINC_G6_STENCIL_OPERATION_KEEP;

	pipeline->stencil_compare_back = KINC_G6_COMPARE_FUNCTION_ALWAYS;
	pipeline->stencil_fail_back = KINC_G6_STENCIL_OPERATION_KEEP;
	pipeline->depth_fail_back = KINC_G6_STENCIL_OPERATION_KEEP;
	pipeline->stencil_pass_back = KINC_G6_STENCIL_OPERATION_KEEP;

	pipeline->stencil_read_mask = 0xFFFFFFFF;
	pipeline->stencil_write_mask = 0xFFFFFFFF;

	pipeline->depth_bias = 0;
	pipeline->depth_bias_slope_scale = 0;
	pipeline->depth_bias_clamp = 0;

	pipeline->sample_count = 1;
	pipeline->sample_mask = 0xFFFFFFFF;
	pipeline->sample_alpha_to_coverage_enabled = false;

	pipeline->attachment_count = 0;

	for (int i = 0; i < 8; i++) {
		pipeline->attachments[i].format = KINC_G6_TEXTURE_FORMAT_NONE;

		pipeline->attachments[i].operation = KINC_G6_BLEND_OPERATION_ADD;
		pipeline->attachments[i].blend_src = KINC_G6_BLEND_FACTOR_ONE;
		pipeline->attachments[i].blend_dst = KINC_G6_BLEND_FACTOR_ZERO;

		pipeline->attachments[i].alpha_operation = KINC_G6_BLEND_OPERATION_ADD;
		pipeline->attachments[i].alpha_blend_src = KINC_G6_BLEND_FACTOR_ONE;
		pipeline->attachments[i].alpha_blend_dst = KINC_G6_BLEND_FACTOR_ZERO;

		pipeline->attachments[i].color_write_mask_red = true;
		pipeline->attachments[i].color_write_mask_green = true;
		pipeline->attachments[i].color_write_mask_blue = true;
		pipeline->attachments[i].color_write_mask_alpha = true;
	}
}