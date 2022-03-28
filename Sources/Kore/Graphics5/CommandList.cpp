#include "CommandList.h"

#include "ConstantBuffer.h"
#include "Graphics.h"
#include "PipelineState.h"

#include <assert.h>

using namespace Kore::Graphics5;

CommandList::CommandList() {
	kinc_g5_command_list_init(&kincCommandList);
}

CommandList::~CommandList() {
	kinc_g5_command_list_destroy(&kincCommandList);
}

void CommandList::begin() {
	kinc_g5_command_list_begin(&kincCommandList);
}

void CommandList::end() {
	kinc_g5_command_list_end(&kincCommandList);
}

void CommandList::clear(RenderTarget *renderTarget, uint flags, uint color, float depth, int stencil) {
	kinc_g5_command_list_clear(&kincCommandList, &renderTarget->kincRenderTarget, flags, color, depth, stencil);
}

void CommandList::renderTargetToFramebufferBarrier(RenderTarget *renderTarget) {
	kinc_g5_command_list_render_target_to_framebuffer_barrier(&kincCommandList, &renderTarget->kincRenderTarget);
}

void CommandList::framebufferToRenderTargetBarrier(RenderTarget *renderTarget) {
	kinc_g5_command_list_framebuffer_to_render_target_barrier(&kincCommandList, &renderTarget->kincRenderTarget);
}

void CommandList::textureToRenderTargetBarrier(RenderTarget *renderTarget) {
	kinc_g5_command_list_texture_to_render_target_barrier(&kincCommandList, &renderTarget->kincRenderTarget);
}

void CommandList::renderTargetToTextureBarrier(RenderTarget *renderTarget) {
	kinc_g5_command_list_render_target_to_texture_barrier(&kincCommandList, &renderTarget->kincRenderTarget);
}

void CommandList::drawIndexedVertices() {
	kinc_g5_command_list_draw_indexed_vertices(&kincCommandList);
}

void CommandList::drawIndexedVertices(int start, int count) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(&kincCommandList, start, count);
}

void CommandList::drawIndexedVerticesInstanced(int instanceCount) {
	kinc_g5_command_list_draw_indexed_vertices_instanced(&kincCommandList, instanceCount);
}

void CommandList::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	kinc_g5_command_list_draw_indexed_vertices_instanced_from_to(&kincCommandList, instanceCount, start, count);
}

void CommandList::viewport(int x, int y, int width, int height) {
	kinc_g5_command_list_viewport(&kincCommandList, x, y, width, height);
}

void CommandList::scissor(int x, int y, int width, int height) {
	kinc_g5_command_list_scissor(&kincCommandList, x, y, width, height);
}

void CommandList::disableScissor() {
	kinc_g5_command_list_disable_scissor(&kincCommandList);
}

void CommandList::setPipeline(PipelineState *pipeline) {
	kinc_g5_command_list_set_pipeline(&kincCommandList, &pipeline->kincPipeline);
}

void CommandList::setVertexBuffers(VertexBuffer **buffers, int *offsets, int count) {
	assert(count <= 16);
	kinc_g5_vertex_buffer_t *kincBuffers[16];
	for (int i = 0; i < count; ++i) {
		kincBuffers[i] = &buffers[i]->kincBuffer;
	}
	kinc_g5_command_list_set_vertex_buffers(&kincCommandList, kincBuffers, offsets, count);
}

void CommandList::setIndexBuffer(IndexBuffer &buffer) {
	kinc_g5_command_list_set_index_buffer(&kincCommandList, &buffer.kincBuffer);
}

void CommandList::setRenderTargets(RenderTarget **targets, int count) {
	assert(count <= 16);
	kinc_g5_render_target_t *kincTargets[16];
	for (int i = 0; i < count; ++i) {
		kincTargets[i] = &targets[i]->kincRenderTarget;
	}
	kinc_g5_command_list_set_render_targets(&kincCommandList, kincTargets, count);
}

void CommandList::upload(IndexBuffer *buffer) {
	kinc_g5_command_list_upload_index_buffer(&kincCommandList, &buffer->kincBuffer);
}

void CommandList::upload(VertexBuffer *buffer) {
	kinc_g5_command_list_upload_vertex_buffer(&kincCommandList, &buffer->kincBuffer);
}

void CommandList::upload(Texture *texture) {
	kinc_g5_command_list_upload_texture(&kincCommandList, &texture->kincTexture);
}

void CommandList::setVertexConstantBuffer(ConstantBuffer *buffer, int offset, size_t size) {
	kinc_g5_command_list_set_vertex_constant_buffer(&kincCommandList, &buffer->kincBuffer, offset, size);
}

void CommandList::setFragmentConstantBuffer(ConstantBuffer *buffer, int offset, size_t size) {
	kinc_g5_command_list_set_fragment_constant_buffer(&kincCommandList, &buffer->kincBuffer, offset, size);
}

void CommandList::setPipelineLayout() {
	kinc_g5_command_list_set_pipeline_layout(&kincCommandList);
}

void CommandList::execute() {
	kinc_g5_command_list_execute(&kincCommandList);
}

void CommandList::executeAndWait() {
	kinc_g5_command_list_execute_and_wait(&kincCommandList);
}

void CommandList::setTexture(TextureUnit unit, Texture *texture) {
	kinc_g5_command_list_set_texture(&kincCommandList, unit.kincTextureUnit, &texture->kincTexture);
}

void CommandList::setImageTexture(TextureUnit unit, Texture *texture) {
	kinc_g5_command_list_set_image_texture(&kincCommandList, unit.kincTextureUnit, &texture->kincTexture);
}

void CommandList::setRenderTargetFace(RenderTarget *texture, int face) {
	kinc_g5_command_list_set_render_target_face(&kincCommandList, &texture->kincRenderTarget, face);
}

void CommandList::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	kinc_g5_command_list_set_texture_addressing(&kincCommandList, unit.kincTextureUnit, (kinc_g5_texture_direction_t)dir,
	                                            (kinc_g5_texture_addressing_t)addressing);
}

void CommandList::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g5_command_list_set_texture_magnification_filter(&kincCommandList, texunit.kincTextureUnit, (kinc_g5_texture_filter_t)filter);
}

void CommandList::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g5_command_list_set_texture_minification_filter(&kincCommandList, texunit.kincTextureUnit, (kinc_g5_texture_filter_t)filter);
}

void CommandList::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	kinc_g5_command_list_set_texture_mipmap_filter(&kincCommandList, texunit.kincTextureUnit, (kinc_g5_mipmap_filter_t)filter);
}

bool CommandList::initOcclusionQuery(uint *occlusionQuery) {
	return kinc_g5_command_list_init_occlusion_query(&kincCommandList, occlusionQuery);
}

void CommandList::deleteOcclusionQuery(uint occlusionQuery) {
	kinc_g5_command_list_delete_occlusion_query(&kincCommandList, occlusionQuery);
}

void CommandList::renderOcclusionQuery(uint occlusionQuery, int triangles) {
	kinc_g5_command_list_render_occlusion_query(&kincCommandList, occlusionQuery, triangles);
}

bool CommandList::isQueryResultsAvailable(uint occlusionQuery) {
	return kinc_g5_command_list_are_query_results_available(&kincCommandList, occlusionQuery);
}

void CommandList::getQueryResults(uint occlusionQuery, uint *pixelCount) {
	kinc_g5_command_list_get_query_result(&kincCommandList, occlusionQuery, pixelCount);
}
