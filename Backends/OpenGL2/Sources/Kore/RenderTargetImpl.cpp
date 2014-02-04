#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>
#include "ogl.h"

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
	
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glBindTexture(GL_TEXTURE_2D, _texture);
}
