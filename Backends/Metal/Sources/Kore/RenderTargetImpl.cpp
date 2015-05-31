#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>

using namespace Kore;

namespace {
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}
	
	int getPower2(int i) {
		for (int power = 0; ; ++power)
			if (pow(power) >= i) return pow(power);
	}
}

RenderTarget::RenderTarget(int width, int height, bool zBuffer, bool antialiasing, RenderTargetFormat format) : width(width), height(height) {
	texWidth = getPower2(width);
	texHeight = getPower2(height);
	
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	
}
