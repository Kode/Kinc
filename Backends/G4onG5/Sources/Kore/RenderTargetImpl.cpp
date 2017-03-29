#include "pch.h"

#include "RenderTargetImpl.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	
}

RenderTarget::~RenderTarget() {

}

void RenderTarget::useColorAsTexture(TextureUnit unit) {

}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {

}

void RenderTarget::setDepthStencilFrom(RenderTarget* source) {

}
