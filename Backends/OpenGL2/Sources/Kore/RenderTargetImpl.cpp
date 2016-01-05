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
#ifndef VR_RIFT
	// TODO: For the DK 2 we need a NPOT texture
	texWidth = getPower2(width);
	texHeight = getPower2(height);
#else 
	texWidth = width;
	texHeight = height;
#endif
	
	glGenTextures(1, &_texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, _texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	
	switch (format) {
	case Target128BitFloat:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, 0);
		break;
	case Target32Bit:
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	
	glCheckErrors();

	glGenFramebuffers(1, &_framebuffer);
	glCheckErrors();
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glCheckErrors();
	
	if (zBuffer) {
		GLuint depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glCheckErrors();
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glCheckErrors();
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);
		glCheckErrors();
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		glCheckErrors();
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
	glCheckErrors();
	//GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, drawBuffers);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, 0);
	glCheckErrors();
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, _texture);
	glCheckErrors();
}
