#include "G4.h"
#include "kinc/graphics4/graphics.h"
#include "kinc/window.h"

#include <kinc/backend/graphics4/indexbuffer.h>
#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/backend/graphics4/vertexbuffer.h>
#include <kinc/color.h>
#include <kinc/compute/compute.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/texturearray.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/io/filereader.h>
#include <kinc/math/core.h>
#include <kinc/math/matrix.h>
#include <kinc/system.h>

#include <assert.h>

kinc_g5_command_list_t commandList;

uint64_t frameNumber = 0;
bool waitAfterNextDraw = false;

static kinc_g5_constant_buffer_t vertexConstantBuffer;
static kinc_g5_constant_buffer_t fragmentConstantBuffer;
#define constantBufferSize 4096
#define constantBufferMultiply 100
static int constantBufferIndex = 0;

void kinc_g4_internal_init() {
	kinc_g5_internal_init();
}

void kinc_g4_internal_destroy() {
	kinc_g5_internal_destroy();
}

void kinc_g4_internal_destroy_window(int window) {
	kinc_g5_internal_destroy_window(window);
}

#define bufferCount 2
#define renderTargetCount 8

static struct {
	int currentBuffer;
	kinc_g5_render_target_t framebuffers[bufferCount];

	kinc_g4_render_target_t *current_render_targets[renderTargetCount];
	int current_render_target_count;

	bool resized;
} windows[16] = {0};

static int current_window;

void kinc_g4_on_g5_internal_resize(int window, int width, int height) {
	windows[window].resized = true;
}

void kinc_g4_internal_init_window(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	kinc_g5_internal_init_window(window, depthBufferBits, stencilBufferBits, vsync);
	kinc_g5_command_list_init(&commandList);
	windows[window].currentBuffer = -1;
	for (int i = 0; i < bufferCount; ++i) {
		kinc_g5_render_target_init_framebuffer(&windows[window].framebuffers[i], kinc_window_width(window), kinc_window_height(window),
		                                       KINC_G5_RENDER_TARGET_FORMAT_32BIT, depthBufferBits, 0);
	}
	kinc_g5_constant_buffer_init(&vertexConstantBuffer, constantBufferSize * constantBufferMultiply);
	kinc_g5_constant_buffer_init(&fragmentConstantBuffer, constantBufferSize * constantBufferMultiply);

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

static void startDraw() {
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);
	kinc_g5_command_list_set_vertex_constant_buffer(&commandList, &vertexConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
	kinc_g5_command_list_set_fragment_constant_buffer(&commandList, &fragmentConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
}

static void endDraw() {
	++constantBufferIndex;
	if (constantBufferIndex >= constantBufferMultiply || waitAfterNextDraw) {
		kinc_g5_command_list_end(&commandList);
		kinc_g5_command_list_execute(&commandList);
		kinc_g5_command_list_wait_for_execution_to_finish(&commandList);
		kinc_g5_command_list_begin(&commandList);
		if (windows[current_window].current_render_targets[0] == NULL) {
			kinc_g4_restore_render_target();
		}
		else {
			const int count = windows[current_window].current_render_target_count;
			kinc_g5_render_target_t *render_targets[16];
			for (int i = 0; i < count; ++i) {
				render_targets[i] = &windows[current_window].current_render_targets[i]->impl._renderTarget;
			}
			kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
		}
		constantBufferIndex = 0;
		waitAfterNextDraw = false;
	}
	kinc_g5_constant_buffer_lock(&vertexConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
	kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, constantBufferIndex * constantBufferSize, constantBufferSize);
}

void kinc_g4_draw_indexed_vertices() {
	startDraw();
	kinc_g5_command_list_draw_indexed_vertices(&commandList);
	endDraw();
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	startDraw();
	kinc_g5_command_list_draw_indexed_vertices_from_to(&commandList, start, count);
	endDraw();
}

void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_offset) {
	startDraw();
	kinc_g5_command_list_draw_indexed_vertices_from_to_from(&commandList, start, count, vertex_offset);
	endDraw();
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	startDraw();
	kinc_g5_command_list_draw_indexed_vertices_instanced(&commandList, instanceCount);
	endDraw();
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
	startDraw();
	kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(&commandList, instanceCount, start, count);
	endDraw();
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	kinc_g5_command_list_set_texture_addressing(&commandList, unit.impl._unit, (kinc_g5_texture_direction_t)dir, (kinc_g5_texture_addressing_t)addressing);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	kinc_g4_set_texture_addressing(unit, dir, addressing);
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	if (windows[current_window].current_render_target_count > 0) {
		if (windows[current_window].current_render_targets[0] == NULL) {
			kinc_g5_command_list_clear(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer], flags, color, depth,
			                           stencil);
		}
		else {
			if (windows[current_window].current_render_targets[0]->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
				kinc_g5_command_list_texture_to_render_target_barrier(&commandList, &windows[current_window].current_render_targets[0]->impl._renderTarget);
				windows[current_window].current_render_targets[0]->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
			}
			kinc_g5_command_list_clear(&commandList, &windows[current_window].current_render_targets[0]->impl._renderTarget, flags, color, depth, stencil);
		}
	}
}

bool first_run = true;

void kinc_g4_begin(int window) {
	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	current_window = window;

	windows[current_window].currentBuffer = (windows[current_window].currentBuffer + 1) % bufferCount;

	bool resized = windows[window].resized;
	if (resized) {
		for (int i = 0; i < bufferCount; ++i) {
			kinc_g5_render_target_destroy(&windows[current_window].framebuffers[i]);
		}
		windows[current_window].currentBuffer = 0;
	}

	kinc_g5_begin(&windows[current_window].framebuffers[windows[current_window].currentBuffer], window);

	if (resized) {
		for (int i = 0; i < bufferCount; ++i) {
			kinc_g5_render_target_init_framebuffer(&windows[current_window].framebuffers[i], kinc_window_width(window), kinc_window_height(window),
			                                       KINC_G5_RENDER_TARGET_FORMAT_32BIT, 16, 0);
		}
		windows[window].resized = false;
	}

	windows[current_window].current_render_targets[0] = NULL;
	windows[current_window].current_render_target_count = 1;

	// commandList = new Graphics5::CommandList;
	kinc_g5_command_list_begin(&commandList);

	// Currently we do not necessarily wait at the end of a frame so for now it's endDraw
	// constantBufferIndex = 0;
	// kinc_g5_constant_buffer_lock(&vertexConstantBuffer, 0, constantBufferSize);
	// kinc_g5_constant_buffer_lock(&fragmentConstantBuffer, 0, constantBufferSize);
	endDraw();

	kinc_g5_command_list_framebuffer_to_render_target_barrier(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer]);
	kinc_g4_restore_render_target();

	++frameNumber;
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	kinc_g5_command_list_viewport(&commandList, x, y, width, height);
}

void kinc_g4_scissor(int x, int y, int width, int height) {
	kinc_g5_command_list_scissor(&commandList, x, y, width, height);
}

void kinc_g4_disable_scissor() {
	kinc_g5_command_list_disable_scissor(&commandList);
}

void kinc_g4_end(int window) {
	kinc_g5_constant_buffer_unlock(&vertexConstantBuffer);
	kinc_g5_constant_buffer_unlock(&fragmentConstantBuffer);

	kinc_g5_command_list_render_target_to_framebuffer_barrier(&commandList, &windows[current_window].framebuffers[windows[current_window].currentBuffer]);
	kinc_g5_command_list_end(&commandList);
	kinc_g5_command_list_execute(&commandList);

	// delete commandList;
	// commandList = nullptr;
	kinc_g5_end(window);

	// to support doing work after kinc_g4_end and before kinc_g4_begin
	kinc_g5_command_list_begin(&commandList);
}

/*void Graphics4::_changeFramebuffer(int window, Kore::FramebufferOptions* frame) {

}*/

bool kinc_g4_swap_buffers() {
	return kinc_g5_swap_buffers();
}

void kinc_g4_flush() {
	kinc_g5_flush();
}

void kinc_g4_set_stencil_reference_value(int value) {}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_int(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_int(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float2(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2);
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float3(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3);
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value1, value2, value3, value4);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_float4(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value1, value2, value3, value4);
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, values, count);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_floats(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, values, count);
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_bool(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix4(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	if (location.impl._location.impl.vertexOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&vertexConstantBuffer, location.impl._location.impl.vertexOffset, value);
	if (location.impl._location.impl.fragmentOffset >= 0)
		kinc_g5_constant_buffer_set_matrix3(&fragmentConstantBuffer, location.impl._location.impl.fragmentOffset, value);
}

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g5_command_list_set_texture_magnification_filter(&commandList, texunit.impl._unit, (kinc_g5_texture_filter_t)filter);
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_magnification_filter(texunit, filter);
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g5_command_list_set_texture_minification_filter(&commandList, texunit.impl._unit, (kinc_g5_texture_filter_t)filter);
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_minification_filter(texunit, filter);
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	kinc_g5_command_list_set_texture_mipmap_filter(&commandList, texunit.impl._unit, (kinc_g5_mipmap_filter_t)filter);
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	kinc_g4_set_texture_mipmap_filter(texunit, filter);
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {}
void kinc_g4_set_texture_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {}
void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {}
void kinc_g4_set_cubemap_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {}
void kinc_g4_set_texture_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {}
void kinc_g4_set_cubemap_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {}
void kinc_g4_set_texture_lod(kinc_g4_texture_unit_t unit, float lod_mind_clamp, float lod_max_clamp);
void kinc_g4_set_cubemap_lod(kinc_g4_texture_unit_t unit, float lod_mind_clamp, float lod_max_clamp);

void kinc_g4_restore_render_target() {
	windows[current_window].current_render_targets[0] = NULL;
	kinc_g5_render_target_t *render_target = &windows[current_window].framebuffers[windows[current_window].currentBuffer];
	kinc_g5_command_list_set_render_targets(&commandList, &render_target, 1);
	windows[current_window].current_render_target_count = 1;
}

void kinc_g4_set_render_targets(kinc_g4_render_target_t **targets, int count) {
	for (int i = 0; i < count; ++i) {
		windows[current_window].current_render_targets[i] = targets[i];
		if (windows[current_window].current_render_targets[i]->impl.state != KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET) {
			kinc_g5_command_list_texture_to_render_target_barrier(&commandList, &windows[current_window].current_render_targets[i]->impl._renderTarget);
			windows[current_window].current_render_targets[i]->impl.state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
		}
	}
	windows[current_window].current_render_target_count = count;
	kinc_g5_render_target_t *render_targets[16];
	assert(count <= 16);
	for (int i = 0; i < count; ++i) {
		render_targets[i] = &targets[i]->impl._renderTarget;
	}
	kinc_g5_command_list_set_render_targets(&commandList, render_targets, count);
}

void kinc_g4_set_render_target_face(kinc_g4_render_target_t *texture, int face) {
	kinc_g5_command_list_set_render_target_face(&commandList, &texture->impl._renderTarget, face);
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count) {
	kinc_g5_vertex_buffer_t *g5buffers[16];
	int offsets[16];
	for (int i = 0; i < count; ++i) {
		g5buffers[i] = &buffers[i]->impl._buffer;
		int index = buffers[i]->impl._currentIndex;
		offsets[i] = index * kinc_g4_vertex_buffer_count(buffers[i]);
	}
	kinc_g5_command_list_set_vertex_buffers(&commandList, g5buffers, offsets, count);
}

int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset) {
	kinc_g4_vertex_buffer_t *buffers[1];
	buffers[0] = buffer;
	kinc_g4_set_vertex_buffers(buffers, 1);
	return 0;
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_g5_command_list_set_index_buffer(&commandList, &buffer->impl._buffer);
}

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	if (!texture->impl._uploaded) {
		kinc_g5_command_list_upload_texture(&commandList, &texture->impl._texture);
		texture->impl._uploaded = true;
	}
	kinc_g5_command_list_set_texture(&commandList, unit.impl._unit, &texture->impl._texture);
}

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {}

int kinc_g4_max_bound_textures(void) {
	return kinc_g5_max_bound_textures();
}

bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery) {
	return kinc_g5_command_list_init_occlusion_query(&commandList, occlusionQuery);
}

void kinc_g4_delete_occlusion_query(unsigned occlusionQuery) {
	kinc_g5_command_list_delete_occlusion_query(&commandList, occlusionQuery);
}

bool kinc_g4_are_query_results_available(unsigned occlusionQuery) {
	return kinc_g5_command_list_are_query_results_available(&commandList, occlusionQuery);
}

void kinc_g4_get_query_result(unsigned occlusionQuery, unsigned *pixelCount) {
	kinc_g5_command_list_get_query_result(&commandList, occlusionQuery, pixelCount);
}

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_g5_command_list_set_pipeline(&commandList, &pipeline->impl._pipeline);
}

void kinc_g4_set_blend_constant(float r, float g, float b, float a) {
	kinc_g5_command_list_set_blend_constant(&commandList, r, g, b, a);
}

void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, struct kinc_g4_texture_array *array) {}

bool kinc_g4_supports_instanced_rendering() {
	return kinc_g5_supports_instanced_rendering();
}

bool kinc_g4_supports_compute_shaders() {
	return kinc_g5_supports_compute_shaders();
}

bool kinc_g4_supports_blend_constants() {
	return kinc_g5_supports_blend_constants();
}

bool kinc_g4_supports_non_pow2_textures() {
	return kinc_g5_supports_non_pow2_textures();
}

bool kinc_g4_render_targets_inverted_y(void) {
	return kinc_g5_render_targets_inverted_y();
}
