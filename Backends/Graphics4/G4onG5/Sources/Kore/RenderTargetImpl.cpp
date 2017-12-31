#include "pch.h"

#include "RenderTargetImpl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Log.h>
#include <Kore/Graphics5/CommandList.h>

using namespace Kore;

extern Graphics5::CommandList* commandList;

Graphics4::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
	: RenderTargetImpl(width, height, depthBufferBits, antialiasing, (Graphics5::RenderTargetFormat)format, stencilBufferBits, contextId) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;
	if (contextId >= 0) {
		commandList->textureToRenderTargetBarrier(&_renderTarget);
		commandList->clear(&_renderTarget, ClearColorFlag);
	}
}

Graphics4::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
	: RenderTargetImpl(cubeMapSize, depthBufferBits, antialiasing, (Graphics5::RenderTargetFormat)format, stencilBufferBits, contextId) {}

Graphics4::RenderTarget::~RenderTarget() {

}

RenderTargetImpl::RenderTargetImpl(int width, int height, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId)
	: _renderTarget(width, height, depthBufferBits, antialiasing, format, stencilBufferBits, contextId) {}

RenderTargetImpl::RenderTargetImpl(int cubeMapSize, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId)
	: _renderTarget(cubeMapSize, depthBufferBits, antialiasing, format, stencilBufferBits, contextId) {}

void Graphics4::RenderTarget::useColorAsTexture(TextureUnit unit) {
	commandList->renderTargetToTextureBarrier(&_renderTarget);
	_renderTarget.useColorAsTexture(unit._unit);
}

void Graphics4::RenderTarget::useDepthAsTexture(TextureUnit unit) {
	_renderTarget.useDepthAsTexture(unit._unit);
}

void Graphics4::RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	_renderTarget.setDepthStencilFrom(&source->_renderTarget);
}

void Graphics4::RenderTarget::getPixels(u8* data) {}

void Graphics4::RenderTarget::generateMipmaps(int levels) {}
