#include "pch.h"

#include "G4.h"

#include "IndexBufferImpl.h"
#include "ProgramImpl.h"
#include "VertexBufferImpl.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>

using namespace Kore;

void Graphics::destroy(int window) {}

void Graphics::init(int window, int depthBufferBits, int stencilBufferBits, bool vsync) {
	
}

void Graphics::changeResolution(int width, int height) {}

void Graphics::setup() {}

void Graphics::makeCurrent(int window) {}

void Graphics::clearCurrent() {}

void Graphics::drawIndexedVertices() {

}

void Graphics::drawIndexedVertices(int start, int count) {

}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {}

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {

}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {}

void Graphics::begin(int window) {

}

void Graphics::viewport(int x, int y, int width, int height) {

}

void Graphics::scissor(int x, int y, int width, int height) {

}

void Graphics::disableScissor() {

}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue,
                                    int readMask, int writeMask) {

}

void Graphics::end(int window) {

}

bool Graphics::vsynced() {
	return true;
}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::swapBuffers(int window) {
	
	return true;
}

void Graphics::flush() {}

void Graphics::setRenderState(RenderState state, bool on) {}

void Graphics::setRenderState(RenderState state, int v) {}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {}

void Graphics::setInt(ConstantLocation location, int value) {

}

void Graphics::setFloat(ConstantLocation location, float value) {

}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {

}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {

}

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {

}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {

}

void Graphics::setFloat4s(ConstantLocation location, float* values, int count) {

}

void Graphics::setBool(ConstantLocation location, bool value) {

}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {

}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {

}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {}

void Graphics::setBlendingModeSeparate(BlendingOperation source, BlendingOperation destination, BlendingOperation alphaSource, BlendingOperation alphaDestination) {}

bool Graphics::renderTargetsInvertedY() {
	return false;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::restoreRenderTarget() {

}

void Graphics::setRenderTarget(RenderTarget* target, int num, int additionalTargets) {

}

void Graphics::setRenderTargetFace(RenderTarget* texture, int face) {
	
}

void Graphics::setVertexBuffers(VertexBuffer** buffers, int count) {

}

void Graphics::setIndexBuffer(IndexBuffer& buffer) {

}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {

}

bool Graphics::initOcclusionQuery(uint* occlusionQuery) {
	return false;
}

void Graphics::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics::getQueryResults(uint occlusionQuery, uint* pixelCount) {}
