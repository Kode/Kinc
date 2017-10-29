#include "pch.h"

#include "RenderTarget5Impl.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Log.h>

using namespace Kore;

Graphics5::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;

}

Graphics5::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	
}

Graphics5::RenderTarget::~RenderTarget() {

}

void Graphics5::RenderTarget::useColorAsTexture(TextureUnit unit) {

}

void Graphics5::RenderTarget::useDepthAsTexture(TextureUnit unit) {

}

void Graphics5::RenderTarget::setDepthStencilFrom(RenderTarget* source) {

}
