#include "pch.h"

#include "Pipeline.h"

void kinc_g4_internal_pipeline_set_defaults(kinc_g4_pipeline_t *state) {
	for (int i = 0; i < 16; ++i) state->input_layout[i] = NULL;
	state->vertex_shader = NULL;
	state->fragment_shader = NULL;
	state->geometry_shader = NULL;
	state->tessellation_control_shader = NULL;
	state->tessellation_evaluation_shader = NULL;

	state->cull_mode = KINC_G4_CULL_NOTHING;

	state->depth_write = false;
	state->depth_mode = KINC_G4_COMPARE_ALWAYS;

	state->stencil_mode = KINC_G4_COMPARE_ALWAYS;
	state->stencil_both_pass = KINC_G4_STENCIL_KEEP;
	state->stencil_depth_fail = KINC_G4_STENCIL_KEEP;
	state->stencil_fail = KINC_G4_STENCIL_KEEP;
	state->stencil_reference_value = 0;
	state->stencil_read_mask = 0xff;
	state->stencil_write_mask = 0xff;

	state->blend_source = KINC_G4_BLEND_ONE;
	state->blend_destination = KINC_G4_BLEND_ZERO;
	// blendOperation = BlendingOperation.Add;
	state->alpha_blend_source = KINC_G4_BLEND_ONE;
	state->alpha_blend_destination = KINC_G4_BLEND_ZERO;
	// alphaBlendOperation = BlendingOperation.Add;

	for (int i = 0; i < 8; ++i) state->color_write_mask_red[i] = true;
	for (int i = 0; i < 8; ++i) state->color_write_mask_green[i] = true;
	for (int i = 0; i < 8; ++i) state->color_write_mask_blue[i] = true;
	for (int i = 0; i < 8; ++i) state->color_write_mask_alpha[i] = true;

	state->conservative_rasterization = false;
}
