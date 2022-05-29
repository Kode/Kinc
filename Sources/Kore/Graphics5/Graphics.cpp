#include "Graphics.h"

#include <kinc/graphics5/graphics.h>

using namespace Kore::Graphics5;

void Kore_Internal_ConvertVertexStructure(kinc_g4_vertex_structure_t *target, const VertexStructure *source);

VertexBuffer::VertexBuffer(int count, const VertexStructure &structure, bool gpuMemory, int instanceDataStepRate) {
	kinc_g4_vertex_structure_t kincStructure;
	kinc_g4_vertex_structure_init(&kincStructure);
	Kore_Internal_ConvertVertexStructure(&kincStructure, &structure);
	kinc_g5_vertex_buffer_init(&kincBuffer, count, &kincStructure, gpuMemory, instanceDataStepRate);
}

VertexBuffer::~VertexBuffer() {
	kinc_g5_vertex_buffer_destroy(&kincBuffer);
}

float *VertexBuffer::lock() {
	return kinc_g5_vertex_buffer_lock_all(&kincBuffer);
}

float *VertexBuffer::lock(int start, int count) {
	return kinc_g5_vertex_buffer_lock(&kincBuffer, start, count);
}

void VertexBuffer::unlock() {
	kinc_g5_vertex_buffer_unlock_all(&kincBuffer);
}

void VertexBuffer::unlock(int count) {
	kinc_g5_vertex_buffer_unlock(&kincBuffer, count);
}

int VertexBuffer::count() {
	return kinc_g5_vertex_buffer_count(&kincBuffer);
}

int VertexBuffer::stride() {
	return kinc_g5_vertex_buffer_stride(&kincBuffer);
}

IndexBuffer::IndexBuffer(int count, bool gpuMemory) {
	kinc_g5_index_buffer_init(&kincBuffer, count, KINC_G5_INDEX_BUFFER_FORMAT_32BIT, gpuMemory);
}

IndexBuffer::~IndexBuffer() {
	kinc_g5_index_buffer_destroy(&kincBuffer);
}

int *IndexBuffer::lock() {
	return kinc_g5_index_buffer_lock(&kincBuffer);
}

void IndexBuffer::unlock() {
	kinc_g5_index_buffer_unlock(&kincBuffer);
}

int IndexBuffer::count() {
	return kinc_g5_index_buffer_count(&kincBuffer);
}

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	kinc_g5_render_target_init(&kincRenderTarget, width, height, depthBufferBits, antialiasing, (kinc_g5_render_target_format_t)format, stencilBufferBits,
	                           contextId);
}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	kinc_g5_render_target_init_cube(&kincRenderTarget, cubeMapSize, depthBufferBits, antialiasing, (kinc_g5_render_target_format_t)format, stencilBufferBits,
	                                contextId);
}

RenderTarget::~RenderTarget() {
	kinc_g5_render_target_destroy(&kincRenderTarget);
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	kinc_g5_render_target_use_color_as_texture(&kincRenderTarget, unit.kincTextureUnit);
}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {
	kinc_g5_render_target_use_depth_as_texture(&kincRenderTarget, unit.kincTextureUnit);
}

void RenderTarget::setDepthStencilFrom(RenderTarget *source) {
	kinc_g5_render_target_set_depth_stencil_from(&kincRenderTarget, &source->kincRenderTarget);
}

int Kore::Graphics5::antialiasingSamples() {
	return kinc_g5_antialiasing_samples();
}

void Kore::Graphics5::setAntialiasingSamples(int samples) {
	kinc_g5_set_antialiasing_samples(samples);
}

bool Kore::Graphics5::renderTargetsInvertedY() {
	return kinc_g5_render_targets_inverted_y();
}

void Kore::Graphics5::begin(RenderTarget *renderTarget, int window) {
	kinc_g5_begin(&renderTarget->kincRenderTarget, window);
}

void Kore::Graphics5::end(int window) {
	kinc_g5_end(window);
}

bool Kore::Graphics5::swapBuffers() {
	return kinc_g5_swap_buffers();
}

int Kore::Graphics5::maxBoundTextures() {
	return kinc_g5_max_bound_textures();
}

bool Kore::Graphics5::nonPow2TexturesSupported() {
	return kinc_g5_non_pow2_textures_qupported();
}

bool Kore::Graphics5::fullscreen = false;

void Kore::Graphics5::flush() {
	kinc_g5_flush();
}
