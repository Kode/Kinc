#include "pch.h"
#include "Metal.h"
#include "VertexBufferImpl.h"
#include <Kore/Application.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <cstdio>
#import <Metal/Metal.h>

using namespace Kore;

namespace {
	//bool fullscreen;
	//TextureFilter minFilters[32];
	//MipmapFilter mipFilters[32];
	//int originalFramebuffer;
	id <MTLBuffer> uniformsBuffer;
}

id getMetalDevice();
id getMetalEncoder();

void Graphics::destroy() {

	System::destroyWindow();
}

#undef CreateWindow

void Graphics::init() {
	System::createWindow();
	id <MTLDevice> device = getMetalDevice();
	uniformsBuffer = [device newBufferWithLength:1024 options:MTLResourceOptionCPUCacheModeDefault];
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
	float* data = (float*)[uniformsBuffer contents];
	for (int i = 0; i < 16; ++i) {
		data[i] = value.data[i];
	}
}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {

}

void Graphics::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {
	id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	
	//[encoder setDepthStencilState:_depthState];
	//[renderEncoder setVertexBuffer:_dynamicConstantBuffer offset:(sizeof(uniforms_t) * _constantDataBufferIndex) atIndex:1 ];
	[encoder setVertexBuffer:uniformsBuffer offset:0 atIndex:1];
	[encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count indexType:MTLIndexTypeUInt32 indexBuffer:IndexBufferImpl::current->mtlBuffer indexBufferOffset:start];
}

void Graphics::swapBuffers() {
	System::swapBuffers();
}

void beginGL();

void Graphics::begin() {
	beginGL();
	
}

void Graphics::end() {
	
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {

}

void Graphics::setRenderState(RenderState state, bool on) {

}

void Graphics::setRenderState(RenderState state, int v) {

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
	return true;
}
