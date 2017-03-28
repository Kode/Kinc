#include "pch.h"

#include "RenderTargetImpl.h"
#include "ogl.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#ifdef SYS_ANDROID
#include <GLContext.h>
#endif
using namespace Kore;

#ifndef GL_RGBA16F_EXT
#define GL_RGBA16F_EXT 0x881A
#endif

#ifndef GL_RGBA32F_EXT
#define GL_RGBA32F_EXT 0x8814
#endif

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

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

	bool nonPow2RenderTargetsSupported() {
#ifdef OPENGLES
#ifdef SYS_ANDROID
		if (ndk_helper::GLContext::GetInstance()->GetGLVersion() >= 3.0)
			return true;
		else
			return false;
#else
		return false;
#endif
#else
		return true;
#endif
	}
}

void RenderTargetImpl::setupDepthStencil(GLenum texType, int depthBufferBits, int stencilBufferBits, int width, int height) {
	if (depthBufferBits > 0 && stencilBufferBits > 0) {
		_hasDepth = true;
#if defined(OPENGLES) && !defined(SYS_PI)
		GLenum internalFormat = GL_DEPTH24_STENCIL8_OES;
#elif defined(SYS_PI)
		GLenum internalFormat = NULL;
#else
		GLenum internalFormat;
		if (depthBufferBits == 24)
			internalFormat = GL_DEPTH24_STENCIL8;
		else
			internalFormat = GL_DEPTH32F_STENCIL8;
#endif
		// Renderbuffer
		// 		glGenRenderbuffers(1, &_depthRenderbuffer);
		// 		glCheckErrors();
		// 		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		// 		glCheckErrors();
		// 		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		// 		glCheckErrors();
		// #ifdef OPENGLES
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// #else
		// 		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// #endif
		// 		glCheckErrors();
		// Texture
		glGenTextures(1, &_depthTexture);
		glCheckErrors();
		glBindTexture(texType, _depthTexture);
		glCheckErrors();
		glTexImage2D(texType, 0, internalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		glCheckErrors();
		glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glCheckErrors();
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		glCheckErrors();
#ifdef OPENGLES
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texType, _depthTexture, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, texType, _depthTexture, 0);
#else
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texType, _depthTexture, 0);
#endif
		glCheckErrors();
	}
	else if (depthBufferBits > 0) {
		_hasDepth = true;
		// Renderbuffer
		// glGenRenderbuffers(1, &_depthRenderbuffer);
		// glCheckErrors();
		// glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
		// glCheckErrors();
		// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		// glCheckErrors();
		// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
		// glCheckErrors();
		// Texture
		glGenTextures(1, &_depthTexture);
		glCheckErrors();
		glBindTexture(texType, _depthTexture);
		glCheckErrors();
		GLint format = depthBufferBits == 16 ? GL_DEPTH_COMPONENT16 : GL_DEPTH_COMPONENT;
		glTexImage2D(texType, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		glCheckErrors();
		glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glCheckErrors();
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		glCheckErrors();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texType, _depthTexture, 0);
		glCheckErrors();
	}
}

RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
    : width(width), height(height), isCubeMap(false), isDepthAttachment(false) {

	_hasDepth = false;

	if (nonPow2RenderTargetsSupported()) {
		texWidth = width;
		texHeight = height;
	}
	else {
		texWidth = getPower2(width);
		texHeight = getPower2(height);
	}

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
#ifdef OPENGLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_EXT, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#endif
		break;
	case Target64BitFloat:
#ifdef OPENGLES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_EXT, texWidth, texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texWidth, texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#endif
		break;
	case Target16BitDepth:
#ifdef OPENGLES
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		break;
	case Target8BitRed:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
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

	setupDepthStencil(GL_TEXTURE_2D, depthBufferBits, stencilBufferBits, texWidth, texHeight);

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
	// GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	// glDrawBuffers(1, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, 0);
	glCheckErrors();
}

RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId)
    : width(cubeMapSize), height(cubeMapSize), isCubeMap(true), isDepthAttachment(false) {

	_hasDepth = false;

	if (nonPow2RenderTargetsSupported()) {
		texWidth = width;
		texHeight = height;
	}
	else {
		texWidth = getPower2(width);
		texHeight = getPower2(height);
	}

	this->contextId = contextId;

	// (DK) required on windows/gl
	Kore::System::makeCurrent(contextId);

	glGenTextures(1, &_texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_CUBE_MAP, _texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheckErrors();

	switch (format) {
	case Target128BitFloat:
#ifdef OPENGLES
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F_EXT, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#else
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, texWidth, texHeight, 0, GL_RGBA, GL_FLOAT, 0);
#endif
		break;
	case Target64BitFloat:
#ifdef OPENGLES
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F_EXT, texWidth, texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#else
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, texWidth, texHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);
#endif
		break;
	case Target16BitDepth:
#ifdef OPENGLES
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
		break;
	case Target32Bit:
	default:
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	glCheckErrors();
	glGenFramebuffers(1, &_framebuffer);
	glCheckErrors();
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glCheckErrors();

	setupDepthStencil(GL_TEXTURE_CUBE_MAP, depthBufferBits, stencilBufferBits, texWidth, texHeight);

	if (format == Target16BitDepth) {
		isDepthAttachment = true;
#ifndef OPENGLES
		glDrawBuffer(GL_NONE);
		glCheckErrors();
		glReadBuffer(GL_NONE);
		glCheckErrors();
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glCheckErrors();
}

RenderTarget::~RenderTarget() {
	{
		GLuint textures[] = { _texture };
		glDeleteTextures(1, textures);
	}
	if (_hasDepth) {
		GLuint textures[] = { _depthTexture };
		glDeleteTextures(1, textures);
	}
	GLuint framebuffers[] = { _framebuffer };
	glDeleteFramebuffers(1, framebuffers);
}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors();
	glBindTexture(isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, _texture);
	glCheckErrors();
}

void RenderTarget::useDepthAsTexture(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors();
	glBindTexture(isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, _depthTexture);
	glCheckErrors();
}

void RenderTarget::setDepthStencilFrom(RenderTarget* source) {
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, source->_depthTexture, 0);
}
