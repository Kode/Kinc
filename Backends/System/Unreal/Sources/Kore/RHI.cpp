#include <Kore/Graphics/Shader.h>
#include <Kore/IndexBufferImpl.h>
#include <Kore/Log.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>

#include "GlobalShader.h"
#include "PixelShaderDeclaration.h"
#include "ShaderParameterUtils.h"
#include "ShaderParameters.h"
#include <Runtime/RHI/Public/DynamicRHI.h>
#include <Runtime/RHI/Public/RHI.h>
#include <Runtime/RHI/Public/RHIResources.h>
#include <Runtime/RHI/Public/RHIStaticStates.h>

using namespace Kore;

void Graphics::destroy(int windowId) {}

void Graphics::changeResolution(int width, int height) {}

void Graphics::init(int windowId, int depthBufferBits, int stencilBufferBits) {}

void Graphics::flush() {}

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilterType filter) {}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilterType filter) {}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {}

void Graphics::makeCurrent(int contextId) {}

void Graphics::clearCurrent() {}

void Graphics::setRenderTarget(RenderTarget *target, int num, int additionalTargets) {}

void Graphics::setRenderTargetFace(RenderTarget *texture, int face) {}

void Graphics::restoreRenderTarget() {}

void Graphics::drawIndexedVertices() {
	FRHICommandListImmediate &commandList = GRHICommandList.GetImmediateCommandList();
	commandList.DrawIndexedPrimitive(IndexBufferImpl::_current->indexBuffer, PT_TriangleList, 0, 0, 3, 0, 1, 1);
}

void Graphics::drawIndexedVertices(int start, int count) {
	FRHICommandListImmediate &commandList = GRHICommandList.GetImmediateCommandList();
	commandList.DrawIndexedPrimitive(IndexBufferImpl::_current->indexBuffer, PT_TriangleList, 0, 0, 3, 0, 1, 1);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {}

void Graphics::clear(uint flags, uint color, float z, int stencil) {}

void Graphics::begin(int windowId) {}

void Graphics::viewport(int x, int y, int width, int height) {}

void Graphics::scissor(int x, int y, int width, int height) {}

void Graphics::disableScissor() {}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue,
                                    int readMask, int writeMask) {}

void Graphics::end(int windowId) {}

bool Graphics::vsynced() {
	return true;
}

unsigned Graphics::refreshRate() {
	return 60;
}

void Graphics::swapBuffers(int windowId) {}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {}

void Graphics::setBlendingModeSeparate(BlendingOperation source, BlendingOperation destination, BlendingOperation alphaSource,
                                       BlendingOperation alphaDestination) {}

void Graphics::setRenderState(RenderState state, bool on) {}

void Graphics::setRenderState(RenderState state, int v) {}

void Graphics::setRenderState(RenderState state, float value) {}

void Graphics::setBool(ConstantLocation position, bool value) {}

void Graphics::setInt(ConstantLocation position, int value) {}

void Graphics::setFloat(ConstantLocation position, float value) {
	FRHICommandListImmediate &commandList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	commandList.SetShaderParameter(VertexShader->GetVertexShader(), position.parameter.GetBufferIndex(), position.parameter.GetBaseIndex(), 4, &value);
}

void Graphics::setFloat2(ConstantLocation position, float value1, float value2) {}

void Graphics::setFloat3(ConstantLocation position, float value1, float value2, float value3) {}

void Graphics::setFloat4(ConstantLocation position, float value1, float value2, float value3, float value4) {}

void Graphics::setFloats(ConstantLocation location, float *values, int count) {}

void Graphics::setFloat4s(ConstantLocation location, float *values, int count) {}

void Graphics::setMatrix(ConstantLocation location, const mat4 &value) {
	FRHICommandListImmediate &commandList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	mat4 value2 = value.Transpose();
	commandList.SetShaderParameter(VertexShader->GetVertexShader(), location.parameter.GetBufferIndex(), location.parameter.GetBaseIndex(), 4 * 16, value.data);
}

void Graphics::setMatrix(ConstantLocation location, const mat3 &value) {
	FRHICommandListImmediate &commandList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	mat3 value2 = value.Transpose();
	float floats[12];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[y * 4 + x] = value.get(y, x);
		}
	}
	commandList.SetShaderParameter(VertexShader->GetVertexShader(), location.parameter.GetBufferIndex(), location.parameter.GetBaseIndex(), 4 * 12, floats);
}

bool Graphics::renderTargetsInvertedY() {
	return false;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::setVertexBuffers(VertexBuffer **buffers, int count) {
	buffers[0]->_set();
}

void Graphics::setIndexBuffer(IndexBuffer &buffer) {
	buffer._set();
}

void Graphics::setTexture(TextureUnit unit, Texture *texture) {
	texture->_set(unit);
}

bool Graphics::initOcclusionQuery(uint *occlusionQuery) {
	return false;
}

void Graphics::deleteOcclusionQuery(uint occlusionQuery) {}

void Graphics::renderOcclusionQuery(uint occlusionQuery, int triangles) {}

bool Graphics::isQueryResultsAvailable(uint occlusionQuery) {
	return false;
}

void Graphics::getQueryResults(uint occlusionQuery, uint *pixelCount) {}
