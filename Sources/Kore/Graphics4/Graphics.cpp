
#ifdef KORE_G4

#include "pch.h"

#include "Graphics.h"
#include "PipelineState.h"

#include <Kinc/Graphics4/Graphics.h>

#include <assert.h>

#include <limits>

using namespace Kore;

int Graphics4::antialiasingSamples() {
	return kinc_g4_antialiasing_samples();
}

void Graphics4::setAntialiasingSamples(int samples) {
	kinc_g4_set_antialiasing_samples(samples);
}

bool Kore::Graphics4::fullscreen = false;

void Graphics4::setFloat2(ConstantLocation position, vec2 value) {
	setFloat2(position, value.x(), value.y());
}

void Graphics4::setFloat3(ConstantLocation position, vec3 value) {
	setFloat3(position, value.x(), value.y(), value.z());
}

void Graphics4::setFloat4(ConstantLocation position, vec4 value) {
	setFloat4(position, value.x(), value.y(), value.z(), value.w());
}

void Graphics4::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
	setVertexBuffers(vertexBuffers, 1);
}

void Graphics4::setRenderTarget(RenderTarget* target) {
	setRenderTargets(&target, 1);
}

void Graphics4::setIndexBuffer(Kore::Graphics4::IndexBuffer& indexBuffer) {
	kinc_g4_set_index_buffer(&indexBuffer.kincBuffer);
}

void Graphics4::setPipeline(Kore::Graphics4::PipelineState* pipeline) {
	kinc_g4_set_pipeline(&pipeline->kincPipeline);
}

void Graphics4::drawIndexedVertices() {
	kinc_g4_draw_indexed_vertices();
}

void Graphics4::drawIndexedVertices(int start, int count) {
	kinc_g4_draw_indexed_vertices_from_to(start, count);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {
	kinc_g4_draw_indexed_vertices_instanced(instanceCount);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, start, count);
}

void Graphics4::begin(int window) {
	kinc_g4_begin(window);
}

void Graphics4::end(int window) {
	kinc_g4_end(window);
}

bool Graphics4::swapBuffers() {
	return kinc_g4_swap_buffers();
}

void Graphics4::clear(unsigned flags, unsigned color, float depth, int stencil) {
	kinc_g4_clear(flags, color, depth, stencil);
}

void Graphics4::viewport(int x, int y, int width, int height) {
	kinc_g4_viewport(x, y, width, height);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	kinc_g4_scissor(x, y, width, height);
}

void Graphics4::disableScissor() {
	kinc_g4_disable_scissor();
}

void Graphics4::init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
	kinc_g4_init(windowId, depthBufferBits, stencilBufferBits, vsync);
}

void Graphics4::destroy(int windowId) {
	kinc_g4_destroy(windowId);
}

void Graphics4::setTexture(Graphics4::TextureUnit unit, Graphics4::Texture* texture) {
	kinc_g4_set_texture(unit.kincUnit, &texture->kincTexture);
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	Kinc_Matrix3x3 matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 3 * 3);
	kinc_g4_set_matrix3(location.kincConstant, &matrix);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	Kinc_Matrix4x4 matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 4 * 4);
	kinc_g4_set_matrix4(location.kincConstant, &matrix);
}

bool Graphics4::renderTargetsInvertedY() {
	return kinc_g4_render_targets_inverted_y();
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	kinc_g4_set_texture_addressing(unit.kincUnit, (Kinc_G4_TextureDirection)dir, (Kinc_G4_TextureAddressing)addressing);
}

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g4_set_texture_magnification_filter(texunit.kincUnit, (Kinc_G4_TextureFilter)filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g4_set_texture_minification_filter(texunit.kincUnit, (Kinc_G4_TextureFilter)filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	kinc_g4_set_texture_mipmap_filter(texunit.kincUnit, (Kinc_G4_MipmapFilter)filter);
}

bool Graphics4::nonPow2TexturesSupported() {
	return kinc_g4_non_pow2_textures_supported();
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	kinc_g4_set_float(location.kincConstant, value);
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	kinc_g4_set_float2(location.kincConstant, value1, value2);
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	kinc_g4_set_float3(location.kincConstant, value1, value2, value3);
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	kinc_g4_set_float4(location.kincConstant, value1, value2, value3, value4);
}

void Graphics4::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
	assert(count <= 16);
	kinc_g4_vertex_buffer_t *buffers[16];
	for (int i = 0; i < count; ++i) {
		buffers[i] = &vertexBuffers[i]->kincBuffer;
	}
	kinc_g4_set_vertex_buffers(buffers, count);
}

void Graphics4::setRenderTargets(RenderTarget** renderTargets, int count) {
	assert(count <= 16);
	Kinc_G4_RenderTarget *targets[16];
	for (int i = 0; i < count; ++i) {
		targets[i] = &renderTargets[i]->kincRenderTarget;
	}
	kinc_g4_set_render_targets(targets, count);
}

void Kinc_Internal_Resize(int window, int width, int height);

void Graphics4::_resize(int window, int width, int height) {
	Kinc_Internal_Resize(window, width, height);
}

void Kinc_Internal_ChangeFramebuffer(int window, _Kinc_FramebufferOptions *frame);

void Graphics4::_changeFramebuffer(int window, _Kinc_FramebufferOptions* frame) {
	Kinc_Internal_ChangeFramebuffer(window, frame);
}

#endif
