#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, bool zBuffer, bool antialiasing, RenderTargetFormat format) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	
}
