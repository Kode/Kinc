#include "pch.h"

#include "G4.h"

#include "IndexBufferImpl.h"
#include "ProgramImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>

using namespace Kore;

void Graphics4::destroy(int window) {
	Graphics5::destroy(window);
}

void Graphics4::init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	Graphics5::init(window, depthBufferBits, stencilBufferBits, vsync);
}

void Graphics4::changeResolution(int width, int height) {
	Graphics5::changeResolution(width, height);
}

void Graphics4::setup() {
	Graphics5::setup();
}

void Graphics4::makeCurrent(int window) {
	Graphics5::makeCurrent(window);
}

void Graphics4::clearCurrent() {
	Graphics5::clearCurrent();
}

void Graphics4::drawIndexedVertices() {
	Graphics5::drawIndexedVertices();
}

void Graphics4::drawIndexedVertices(int start, int count) {
	Graphics5::drawIndexedVertices(start, count);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount) {
	Graphics5::drawIndexedVerticesInstanced(instanceCount);
}

void Graphics4::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	Graphics5::drawIndexedVerticesInstanced(instanceCount, start, count);
}

void Graphics4::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Graphics5::setTextureAddressing(unit._unit, (Graphics5::TexDir)dir, (Graphics5::TextureAddressing)addressing);
}

void Graphics4::setTexture3DAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	Graphics4::setTextureAddressing(unit, dir, addressing);
}

void Graphics4::setColorMask(bool red, bool green, bool blue, bool alpha) {
	Graphics5::setColorMask(red, green, blue, alpha);
}

void Graphics4::clear(uint flags, uint color, float depth, int stencil) {
	Graphics5::clear(flags, color, depth, stencil);
}

void Graphics4::begin(int window) {
	Graphics5::begin(window);
}

void Graphics4::viewport(int x, int y, int width, int height) {
	Graphics5::viewport(x, y, width, height);
}

void Graphics4::scissor(int x, int y, int width, int height) {
	Graphics5::scissor(x, y, width, height);
}

void Graphics4::disableScissor() {
	Graphics5::disableScissor();
}

void Graphics4::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue,
                                    int readMask, int writeMask) {
	Graphics5::setStencilParameters((Graphics5::ZCompareMode)compareMode, (Graphics5::StencilAction)bothPass, (Graphics5::StencilAction)depthFail, (Graphics5::StencilAction)stencilFail, referenceValue, readMask, writeMask);
}

void Graphics4::end(int window) {
	Graphics5::end(window);
}

bool Graphics4::vsynced() {
	return Graphics5::vsynced();
}

unsigned Graphics4::refreshRate() {
	return Graphics5::refreshRate();
}

bool Graphics4::swapBuffers(int window) {
	return Graphics5::swapBuffers(window);
}

void Graphics4::flush() {
	return Graphics5::flush();
}

void Graphics4::setRenderState(RenderState state, bool on) {
	Graphics5::setRenderState((Graphics5::RenderState)state, on);
}

void Graphics4::setRenderState(RenderState state, int v) {
	Graphics5::setRenderState((Graphics5::RenderState)state, v);
}

void Graphics4::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	Graphics5::setTextureOperation((Graphics5::TextureOperation)operation, (Graphics5::TextureArgument)arg1, (Graphics5::TextureArgument)arg2);
}

void Graphics4::setInt(ConstantLocation location, int value) {
	Graphics5::setInt(location._location, value);
}

void Graphics4::setFloat(ConstantLocation location, float value) {
	Graphics5::setFloat(location._location, value);
}

void Graphics4::setFloat2(ConstantLocation location, float value1, float value2) {
	Graphics5::setFloat2(location._location, value1, value2);
}

void Graphics4::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	Graphics5::setFloat3(location._location, value1, value2, value3);
}

void Graphics4::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	Graphics5::setFloat4(location._location, value1, value2, value3, value4);
}

void Graphics4::setFloats(ConstantLocation location, float* values, int count) {
	Graphics5::setFloats(location._location, values, count);
}

void Graphics4::setBool(ConstantLocation location, bool value) {
	Graphics5::setBool(location._location, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat4& value) {
	Graphics5::setMatrix(location._location, value);
}

void Graphics4::setMatrix(ConstantLocation location, const mat3& value) {
	Graphics5::setMatrix(location._location, value);
}

void Graphics4::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics5::setTextureMagnificationFilter(texunit._unit, (Graphics5::TextureFilter)filter);
}

void Graphics4::setTexture3DMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMagnificationFilter(texunit, filter);
}

void Graphics4::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics5::setTextureMinificationFilter(texunit._unit, (Graphics5::TextureFilter)filter);
}

void Graphics4::setTexture3DMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	Graphics4::setTextureMinificationFilter(texunit, filter);
}

void Graphics4::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Graphics5::setTextureMipmapFilter(texunit._unit, (Graphics5::MipmapFilter)filter);
}

void Graphics4::setTexture3DMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	Graphics4::setTextureMipmapFilter(texunit, filter);
}

void Graphics4::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	Graphics5::setBlendingMode((Graphics5::BlendingOperation)source, (Graphics5::BlendingOperation)destination);
}

void Graphics4::setBlendingModeSeparate(BlendingOperation source, BlendingOperation destination, BlendingOperation alphaSource, BlendingOperation alphaDestination) {
	Graphics5::setBlendingModeSeparate((Graphics5::BlendingOperation)source, (Graphics5::BlendingOperation)destination, (Graphics5::BlendingOperation)alphaSource, (Graphics5::BlendingOperation)alphaDestination);
}

bool Graphics4::renderTargetsInvertedY() {
	return Graphics5::renderTargetsInvertedY();
}

bool Graphics4::nonPow2TexturesSupported() {
	return Graphics5::nonPow2TexturesSupported();
}

void Graphics4::restoreRenderTarget() {
	Graphics5::restoreRenderTarget();
}

void Graphics4::setRenderTarget(RenderTarget* target, int num, int additionalTargets) {
	Graphics5::setRenderTarget(&target->_renderTarget, num, additionalTargets);
}

void Graphics4::setRenderTargetFace(RenderTarget* texture, int face) {
	Graphics5::setRenderTargetFace(&texture->_renderTarget, face);
}

void Graphics4::setVertexBuffers(VertexBuffer** buffers, int count) {
	Graphics5::VertexBuffer* g5buffers[16];
	for (int i = 0; i < count; ++i) {
		g5buffers[i] = &buffers[i]->_buffer;
	}
	Graphics5::setVertexBuffers(g5buffers, count);
}

void Graphics4::setIndexBuffer(IndexBuffer& buffer) {
	Graphics5::setIndexBuffer(buffer._buffer);
}

void Graphics4::setTexture(TextureUnit unit, Texture* texture) {
	Graphics5::setTexture(unit._unit, texture->_texture);
}

bool Graphics4::initOcclusionQuery(uint* occlusionQuery) {
	return Graphics5::initOcclusionQuery(occlusionQuery);
}

void Graphics4::deleteOcclusionQuery(uint occlusionQuery) {
	Graphics5::deleteOcclusionQuery(occlusionQuery);
}

void Graphics4::renderOcclusionQuery(uint occlusionQuery, int triangles) {
	Graphics5::renderOcclusionQuery(occlusionQuery, triangles);
}

bool Graphics4::isQueryResultsAvailable(uint occlusionQuery) {
	return Graphics5::isQueryResultsAvailable(occlusionQuery);
}

void Graphics4::getQueryResults(uint occlusionQuery, uint* pixelCount) {
	Graphics5::getQueryResults(occlusionQuery, pixelCount);
}
