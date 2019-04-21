
#ifdef KORE_G4

#include "pch.h"

#include "Graphics.h"
#include "PipelineState.h"

#include <Kinc/Graphics4/Graphics.h>

#include <assert.h>

#include <limits>

using namespace Kore;

int Graphics4::antialiasingSamples() {
	return Kinc_G4_AntialiasingSamples();
}

void Graphics4::setAntialiasingSamples(int samples) {
	Kinc_G4_SetAntialiasingSamples(samples);
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
	Kinc_G4_SetIndexBuffer(&indexBuffer.kincBuffer);
}

void Graphics4::setPipeline(Kore::Graphics4::PipelineState* pipeline) {
	Kinc_G4_SetPipeline(&pipeline->kincPipeline);
}

void Graphics4::drawIndexedVertices() {
	Kinc_G4_DrawIndexedVertices();
}

void Graphics4::drawIndexedVertices(int start, int count) {
	Kinc_G4_DrawIndexedVerticesFromTo(start, count);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {
	Kinc_G4_DrawIndexedVerticesInstanced(instanceCount);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	Kinc_G4_DrawIndexedVerticesInstancedFromTo(instanceCount, start, count);
}

void Graphics4::begin(int window) {
	Kinc_G4_Begin(window);
}

void Graphics4::end(int window) {
	Kinc_G4_End(window);
}

bool Graphics4::swapBuffers() {
	return Kinc_G4_SwapBuffers();
}

void Graphics4::clear(unsigned flags, unsigned color, float depth, int stencil) {
	Kinc_G4_Clear(flags, color, depth, stencil);
}

void Graphics4::viewport(int x, int y, int width, int height) {
	Kinc_G4_Viewport(x, y, width, height);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	Kinc_G4_Scissor(x, y, width, height);
}

void Graphics4::disableScissor() {
	Kinc_G4_DisableScissor();
}

void Graphics4::init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
	Kinc_G4_Init(windowId, depthBufferBits, stencilBufferBits, vsync);
}

void Graphics4::destroy(int windowId) {
	Kinc_G4_Destroy(windowId);
}

void Graphics4::setTexture(Graphics4::TextureUnit unit, Graphics4::Texture* texture) {
	Kinc_G4_SetTexture(unit.kincUnit, &texture->kincTexture);
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	Kinc_Matrix3x3 matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 3 * 3);
	Kinc_G4_SetMatrix3(location.kincConstant, &matrix);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	Kinc_Matrix4x4 matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 4 * 4);
	Kinc_G4_SetMatrix4(location.kincConstant, &matrix);
}

bool Graphics4::renderTargetsInvertedY() {
	return Kinc_G4_RenderTargetsInvertedY();
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Kinc_G4_SetTextureAddressing(unit.kincUnit, (Kinc_G4_TextureDirection)dir, (Kinc_G4_TextureAddressing)addressing);
}

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Kinc_G4_SetTextureMagnificationFilter(texunit.kincUnit, (Kinc_G4_TextureFilter)filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Kinc_G4_SetTextureMinificationFilter(texunit.kincUnit, (Kinc_G4_TextureFilter)filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Kinc_G4_SetTextureMipmapFilter(texunit.kincUnit, (Kinc_G4_MipmapFilter)filter);
}

bool Graphics4::nonPow2TexturesSupported() {
	return Kinc_G4_NonPow2TexturesSupported();
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	Kinc_G4_SetFloat(location.kincConstant, value);
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	Kinc_G4_SetFloat2(location.kincConstant, value1, value2);
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	Kinc_G4_SetFloat3(location.kincConstant, value1, value2, value3);
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	Kinc_G4_SetFloat4(location.kincConstant, value1, value2, value3, value4);
}

void Graphics4::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
	assert(count <= 16);
	Kinc_G4_VertexBuffer *buffers[16];
	for (int i = 0; i < count; ++i) {
		buffers[i] = &vertexBuffers[i]->kincBuffer;
	}
	Kinc_G4_SetVertexBuffers(buffers, count);
}

void Graphics4::setRenderTargets(RenderTarget** renderTargets, int count) {
	assert(count <= 16);
	Kinc_G4_RenderTarget *targets[16];
	for (int i = 0; i < count; ++i) {
		targets[i] = &renderTargets[i]->kincRenderTarget;
	}
	Kinc_G4_SetRenderTargets(targets, count);
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
