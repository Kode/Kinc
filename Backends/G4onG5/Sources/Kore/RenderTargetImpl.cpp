#include "pch.h"

#include "RenderTargetImpl.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
	: RenderTargetImpl(width, height, depthBufferBits, antialiasing, (Graphics5::RenderTargetFormat)format, stencilBufferBits, contextId) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
	: RenderTargetImpl(cubeMapSize, depthBufferBits, antialiasing, (Graphics5::RenderTargetFormat)format, stencilBufferBits, contextId) {}

RenderTarget::~RenderTarget() {

}

RenderTargetImpl::RenderTargetImpl(int width, int height, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId)
	: _renderTarget(width, height, depthBufferBits, antialiasing, format, stencilBufferBits, contextId) {}

RenderTargetImpl::RenderTargetImpl(int cubeMapSize, int depthBufferBits, bool antialiasing, Graphics5::RenderTargetFormat format, int stencilBufferBits, int contextId)
	: _renderTarget(cubeMapSize, depthBufferBits, antialiasing, format, stencilBufferBits, contextId) {}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	_renderTarget.useColorAsTexture(unit._unit);
}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {
	_renderTarget.useDepthAsTexture(unit._unit);
}

void RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	_renderTarget.setDepthStencilFrom(&source->_renderTarget);
}
