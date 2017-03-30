#include "pch.h"

#include "RenderTargetImpl.h"

#include <Kore/Log.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
    : width(width), height(height), texWidth(width), texHeight(height) {

}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	
}

RenderTarget::~RenderTarget() {

}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	
}

void RenderTarget::setDepthStencilFrom(RenderTarget* source) {

}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {}
