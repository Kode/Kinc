#include "pch.h"

#include "RenderTarget5Impl.h"

#include <Kore/Graphics5/Graphics.h>

using namespace Kore;

namespace {
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0;; ++power)
			if (pow(power) >= i) return pow(power);
	}
}

Graphics5::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
    : width(width), height(height) {
	texWidth = getPower2(width);
	texHeight = getPower2(height);
}

Graphics5::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {
	
}

Graphics5::RenderTarget::~RenderTarget() {}

void Graphics5::RenderTarget::useColorAsTexture(TextureUnit unit) {}

void Graphics5::RenderTarget::useDepthAsTexture(Graphics5::TextureUnit unit) {}

void Graphics5::RenderTarget::setDepthStencilFrom(Graphics5::RenderTarget* source) {}
