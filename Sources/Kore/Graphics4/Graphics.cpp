
#ifdef KORE_G4

#include "pch.h"

#include "Graphics.h"
#include "PipelineState.h"

#include <kinc/graphics4/graphics.h>

#include <Kore/Graphics4/TextureArray.h>

#include <assert.h>
#include <memory.h>

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

void Graphics4::setInt2(ConstantLocation position, vec2i value) {
	setInt2(position, value.x(), value.y());
}

void Graphics4::setInt3(ConstantLocation position, vec3i value) {
	setInt3(position, value.x(), value.y(), value.z());
}

void Graphics4::setInt4(ConstantLocation position, vec4i value) {
	setInt4(position, value.x(), value.y(), value.z(), value.w());
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
	kinc_matrix3x3_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 3 * 3);
	kinc_g4_set_matrix3(location.kincConstant, &matrix);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	kinc_matrix4x4_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 4 * 4);
	kinc_g4_set_matrix4(location.kincConstant, &matrix);
}

bool Graphics4::renderTargetsInvertedY() {
	return kinc_g4_render_targets_inverted_y();
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	kinc_g4_set_texture_addressing(unit.kincUnit, (kinc_g4_texture_direction_t)dir, (kinc_g4_texture_addressing_t)addressing);
}

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g4_set_texture_magnification_filter(texunit.kincUnit, (kinc_g4_texture_filter_t)filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	kinc_g4_set_texture_minification_filter(texunit.kincUnit, (kinc_g4_texture_filter_t)filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	kinc_g4_set_texture_mipmap_filter(texunit.kincUnit, (kinc_g4_mipmap_filter_t)filter);
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

void Graphics4::setFloats(ConstantLocation location, float* values, int count) {
	kinc_g4_set_floats(location.kincConstant, values, count);
}

void Graphics4::setInt(ConstantLocation location, int value) {
	kinc_g4_set_int(location.kincConstant, value);
}

void Graphics4::setInt2(ConstantLocation location, int value1, int value2) {
	kinc_g4_set_int2(location.kincConstant, value1, value2);
}

void Graphics4::setInt3(ConstantLocation location, int value1, int value2, int value3) {
	kinc_g4_set_int3(location.kincConstant, value1, value2, value3);
}

void Graphics4::setInt4(ConstantLocation location, int value1, int value2, int value3, int value4) {
	kinc_g4_set_int4(location.kincConstant, value1, value2, value3, value4);
}

void Graphics4::setInts(ConstantLocation location, int* values, int count) {
	kinc_g4_set_ints(location.kincConstant, values, count);
}

void Graphics4::setBool(ConstantLocation location, bool value) {
	kinc_g4_set_bool(location.kincConstant, value);
}

void Graphics4::flush() {
	kinc_g4_flush();
}

void Graphics4::restoreRenderTarget() {
	kinc_g4_restore_render_target();
}

void Graphics4::setStencilReferenceValue(int value) {
	kinc_g4_set_stencil_reference_value(value);
}

void Graphics4::setTextureArray(TextureUnit unit, TextureArray* array) {
	kinc_g4_set_texture_array(unit.kincUnit, &array->kincArray);
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
	kinc_g4_render_target_t *targets[16];
	for (int i = 0; i < count; ++i) {
		targets[i] = &renderTargets[i]->kincRenderTarget;
	}
	kinc_g4_set_render_targets(targets, count);
}

void Graphics4::setImageTexture(TextureUnit unit, Texture* texture) {
	kinc_g4_set_image_texture(unit.kincUnit, &texture->kincTexture);
}

void Graphics4::setTextureCompareMode(TextureUnit unit, bool enabled) {
	kinc_g4_set_texture_compare_mode(unit.kincUnit, enabled);
}

void Graphics4::setCubeMapCompareMode(TextureUnit unit, bool enabled) {
	kinc_g4_set_cubemap_compare_mode(unit.kincUnit, enabled);
}

void Graphics4::setRenderTargetFace(RenderTarget* renderTarget, int face) {
	kinc_g4_set_render_target_face(&renderTarget->kincRenderTarget, face);
}

void Graphics4::setTexture3DAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	kinc_g4_set_texture3d_addressing(unit.kincUnit, (kinc_g4_texture_direction_t)dir, (kinc_g4_texture_addressing_t)addressing);
}
void Graphics4::setTexture3DMagnificationFilter(TextureUnit unit, TextureFilter filter) {
	kinc_g4_set_texture3d_magnification_filter(unit.kincUnit, (kinc_g4_texture_filter_t)filter);
}
void Graphics4::setTexture3DMinificationFilter(TextureUnit unit, TextureFilter filter) {
	kinc_g4_set_texture3d_minification_filter(unit.kincUnit, (kinc_g4_texture_filter_t)filter);
}
void Graphics4::setTexture3DMipmapFilter(TextureUnit unit, MipmapFilter filter) {
	kinc_g4_set_texture3d_mipmap_filter(unit.kincUnit, (kinc_g4_mipmap_filter_t)filter);
}

extern "C" void kinc_internal_resize(int window, int width, int height);

void Graphics4::_resize(int window, int width, int height) {
	kinc_internal_resize(window, width, height);
}

extern "C" void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame);

void Graphics4::_changeFramebuffer(int window, struct kinc_framebuffer_options *frame) {
	kinc_internal_change_framebuffer(window, frame);
}

#endif
