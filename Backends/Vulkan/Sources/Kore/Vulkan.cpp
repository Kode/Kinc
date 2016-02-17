#include "pch.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Application.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <Kore/Log.h>
#include <cstdio>
#include <vulkan/vulkan.h>

using namespace Kore;

namespace {

}

void Graphics::destroy() {

}

void Graphics::init() {

}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::vsynced() {
	return true;
}

void* Graphics::getControl() {
	return nullptr;
}

void Graphics::setBool(ConstantLocation location, bool value) {

}

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

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {

}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {

}

void Graphics::drawIndexedVertices() {
	//drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {

}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {
	//drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {

}

void Graphics::swapBuffers() {

}

void Graphics::begin() {

}

void Graphics::viewport(int x, int y, int width, int height) {

}

void Graphics::scissor(int x, int y, int width, int height) {

}

void Graphics::disableScissor() {

}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {

}

void Graphics::end() {

}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {

}

void Graphics::setRenderState(RenderState state, bool on) {

}

void Graphics::setRenderState(RenderState state, int v) {
	
}

void Graphics::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {

}

void Graphics::setIndexBuffer(IndexBuffer& indexBuffer) {

}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {

}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {

}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {

}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {

}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {

}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {

}

void Graphics::setRenderTarget(RenderTarget* texture, int num) {

}

void Graphics::restoreRenderTarget() {

}

bool Graphics::renderTargetsInvertedY() {
	return true;
}

bool Graphics::nonPow2TexturesSupported() {
	return false;
}

void Graphics::flush() {

}
