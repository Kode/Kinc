#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
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

	void setupDepthStencil(int depthBufferBits, int stencilBufferBits, int width, int height) {
		if (depthBufferBits > 0 && stencilBufferBits > 0) {
#ifdef OPENGLES
			GLenum internalFormat = GL_DEPTH24_STENCIL8_OES;
#else
			GLenum internalFormat;
			if (depthBufferBits == 24) internalFormat = GL_DEPTH24_STENCIL8;
			else internalFormat = GL_DEPTH32F_STENCIL8;
#endif
			GLuint dsBuffer;
			glGenRenderbuffers(1, &dsBuffer);
			glCheckErrors();
			glBindRenderbuffer(GL_RENDERBUFFER, dsBuffer);
			glCheckErrors();
			glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
			glCheckErrors();
#ifdef OPENGLES
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dsBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsBuffer);
#else
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsBuffer);
#endif
			glCheckErrors();
		}
		else if (depthBufferBits > 0) {
			GLuint depthBuffer;
			glGenRenderbuffers(1, &depthBuffer);
			glCheckErrors();
			glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
			glCheckErrors();
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glCheckErrors();
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
			glCheckErrors();
		}
	}
}

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) : width(width), height(height) {
#ifndef VR_RIFT
	// TODO: For the DK 2 we need a NPOT texture
	texWidth = getPower2(width);
	texHeight = getPower2(height);
#else
	texWidth = width;
	texHeight = height;
#endif

	this->contextId = contextId;

	// (DK) required on windows/gl
	Kore::System::makeCurrent(contextId);

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
	case Target64BitFloat:
#ifndef OPENGLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#endif
		break;
	case Target16BitDepth:
#ifdef OPENGLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
#else // GL, GLES3
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
#endif
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

	setupDepthStencil(depthBufferBits, stencilBufferBits, texWidth, texHeight);

	if (format == Target16BitDepth) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _texture, 0);
#ifndef OPENGLES
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
#endif
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
	}
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
