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
}

void Graphics::destroy() {

	System::destroyWindow();
}

#undef CreateWindow

void Graphics::init() {
	System::createWindow();

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

void* getMetalDevice();

void Graphics::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {
	id <MTLDevice> device = (__bridge_transfer id <MTLDevice>)getMetalDevice();
	
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
