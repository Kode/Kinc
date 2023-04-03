#pragma once

struct kinc_g5_index_buffer;
struct kinc_g5_render_target;
struct kinc_g5_pipeline;

typedef struct {
	void *commandBuffer;
	void *commandEncoder;
	struct kinc_g5_index_buffer *current_index_buffer;
	struct kinc_g5_render_target *lastRenderTargets[8];
	struct kinc_g5_pipeline *lastPipeline;
} CommandList5Impl;
