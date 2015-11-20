#include "pch.h"
#include "OpenGL.h"
#include "VertexBufferImpl.h"
#include <Kore/Application.h>
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <Kore/Log.h>
#include "ogl.h"
#include <cstdio>

#ifdef SYS_WINDOWS

#include <GL/wglew.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#endif

using namespace Kore;

namespace Kore {
#if !defined(SYS_IOS) && !defined(SYS_ANDROID)
	extern bool programUsesTesselation;
#endif
}

namespace {
#ifdef SYS_WINDOWS
	HINSTANCE instance = 0;
	HWND windowHandle = 0;
	HDC deviceContext = 0;
	HGLRC glContext = 0;
#endif
	//bool fullscreen;
	TextureFilter minFilters[32];
	MipmapFilter mipFilters[32];
	int originalFramebuffer;
	uint arrayId;
}

void Graphics::destroy() {
#ifdef SYS_WINDOWS
	if (glContext) {
		if (!wglMakeCurrent(nullptr, nullptr)) {
			//MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(glContext)) {
			//MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		glContext = nullptr;
	}

	if (deviceContext && !ReleaseDC(windowHandle, deviceContext)) {
		//MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		deviceContext = nullptr;
	}
#endif
	System::destroyWindow();
}

#undef CreateWindow

void Graphics::init() {
#ifdef SYS_WINDOWS
	windowHandle = (HWND)System::createWindow();

#ifndef VR_RIFT
	PIXELFORMATDESCRIPTOR pfd =					// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,								// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,						// Must Support Double Buffering
		PFD_TYPE_RGBA,							// Request An RGBA Format
		32,								// Select Our Color Depth
		0, 0, 0, 0, 0, 0,						// Color Bits Ignored
		0,								// No Alpha Buffer
		0,								// Shift Bit Ignored
		0,								// No Accumulation Buffer
		0, 0, 0, 0,							// Accumulation Bits Ignored
		16,								// 16Bit Z-Buffer (Depth Buffer)
		0,								// No Stencil Buffer
		0,								// No Auxiliary Buffer
		PFD_MAIN_PLANE,							// Main Drawing Layer
		0,								// Reserved
		0, 0, 0								// Layer Masks Ignored
	};

	deviceContext = GetDC(windowHandle);
	GLuint pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
	SetPixelFormat(deviceContext, pixelFormat, &pfd);
	HGLRC tempGlContext = wglCreateContext(deviceContext);
	wglMakeCurrent(deviceContext, tempGlContext);

	glewInit();

	if (wglewIsSupported("WGL_ARB_create_context") == 1) {
		int attributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		glContext = wglCreateContextAttribsARB(deviceContext, nullptr, attributes);
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tempGlContext);
		wglMakeCurrent(deviceContext, glContext);
	}
	else {
		glContext = tempGlContext;
	}

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle); // Slightly Higher Priority
	SetFocus(windowHandle); // Sets Keyboard Focus To The Window
#else 
	deviceContext = GetDC(windowHandle);
	glContext = wglGetCurrentContext();
	glewInit();
	

#endif
#else
	System::createWindow();
#endif
	
#ifndef VR_RIFT
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setRenderState(DepthTest, false);
	glViewport(0, 0, System::screenWidth(), System::screenHeight());
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFramebuffer);

	for (int i = 0; i < 32; ++i) {
		minFilters[i] = LinearFilter;
		mipFilters[i] = NoMipFilter;
	}
#endif

#ifdef SYS_WINDOWS
	if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(1);
#endif

#if defined(SYS_IOS)
	glGenVertexArraysOES(1, &arrayId);
	glCheckErrors();
#elif !defined(SYS_ANDROID) && !defined(SYS_HTML5) && !defined(SYS_TIZEN)
	glGenVertexArrays(1, &arrayId);
	glCheckErrors();
#endif
}

unsigned Graphics::refreshRate() {
	return 60;
}

bool Graphics::vsynced() {
	return true;
}

void* Graphics::getControl() {
#ifdef SYS_WINDOWS
	return windowHandle;
#else
	return nullptr;
#endif
}

void Graphics::setBool(ConstantLocation location, bool value) {
	glUniform1i(location.location, value ? 1 : 0);
	glCheckErrors();
}

void Graphics::setInt(ConstantLocation location, int value) {
	glUniform1i(location.location, value);
	glCheckErrors();
}

void Graphics::setFloat(ConstantLocation location, float value) {
	glUniform1f(location.location, value);
	glCheckErrors();
}

void Graphics::setFloat2(ConstantLocation location, float value1, float value2) {
	glUniform2f(location.location, value1, value2);
	glCheckErrors();
}

void Graphics::setFloat3(ConstantLocation location, float value1, float value2, float value3) {
	glUniform3f(location.location, value1, value2, value3);
	glCheckErrors();
}

void Graphics::setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4) {
	glUniform4f(location.location, value1, value2, value3, value4);
	glCheckErrors();
}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {
	glUniform1fv(location.location, count, values);
	glCheckErrors();
}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
	glUniformMatrix4fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors();
}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {
	glUniformMatrix3fv(location.location, 1, GL_FALSE, &value.matrix[0][0]);
	glCheckErrors();
}

void Graphics::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVertices(int start, int count) {
#ifdef OPENGLES
#ifdef SYS_ANDROID
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void*)(start * sizeof(GL_UNSIGNED_SHORT)));
#else
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
#endif
#else
	if (programUsesTesselation) {
		glDrawElements(GL_PATCHES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
	else {
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}	
#endif
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {
	drawIndexedVerticesInstanced(instanceCount, 0, IndexBufferImpl::current->count());
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
#ifndef OPENGLES
	int indices[3] = { 0, 1, 2 };
	if (programUsesTesselation) {
		glDrawElementsInstanced(GL_PATCHES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
	else {
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)), instanceCount);
		glCheckErrors();
	}
#endif
}

void Graphics::swapBuffers() {
#ifdef SYS_WINDOWS
	::SwapBuffers(deviceContext);
#else
	System::swapBuffers();
#endif
}

#ifdef SYS_IOS
void beginGL();
#endif

void Graphics::begin() {
#ifdef SYS_IOS
	beginGL();
#endif
#ifdef SYS_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void Graphics::viewport(int x, int y, int width, int height) {
	glViewport(x,y,width,height);
}

void glCheckErrors() {
#ifdef _DEBUG
	GLenum code = glGetError();
	while (code != GL_NO_ERROR) {
		//std::printf("GLError: %s\n", glewGetErrorString(code));
		switch (code) {
		case 1281:
			log(Warning, "OpenGL: Invalid value");
			break;
		case 1282:
			log(Warning, "OpenGL: Invalid operation");
			break;
		default:
			log(Warning, "OpenGL: Error code %i", code);
			break;
		}
		code = glGetError();
	}
#endif
}

void Graphics::end() {
	//glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	glCheckErrors();
}

void Graphics::clear(uint flags, uint color, float depth, int stencil) {
	glClearColor(((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f, (color & 0xff000000) / 255.0f);
#ifdef OPENGLES
	glClearDepthf(depth);
#else
	glClearDepth(depth);
#endif
	glClearStencil(stencil);
	GLbitfield oglflags =
		  ((flags & ClearColorFlag) ? GL_COLOR_BUFFER_BIT : 0)
		| ((flags & ClearDepthFlag) ? GL_DEPTH_BUFFER_BIT : 0)
		| ((flags & ClearStencilFlag) ? GL_STENCIL_BUFFER_BIT: 0);
	glClear(oglflags);
}

void Graphics::setRenderState(RenderState state, bool on) {
	switch (state) {
	case DepthWrite:
		if (on) glDepthMask(GL_TRUE);
		else glDepthMask(GL_FALSE);
		break;
	case DepthTest:
		if (on) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
		break;
	case BlendingState:
		if (on) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);
		break;
	default:
		break;
	}

	/*switch (state) {
		case Normalize:
			device->SetRenderState(D3DRS_NORMALIZENORMALS, on ? TRUE : FALSE);
			break;
		case BackfaceCulling:
			if (on) device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			else device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			break;
		case FogState:
			device->SetRenderState(D3DRS_FOGENABLE, on ? TRUE : FALSE);
			break;
		case ScissorTestState:
			device->SetRenderState(D3DRS_SCISSORTESTENABLE, on ? TRUE : FALSE);
			break;
		case AlphaTestState:
			device->SetRenderState(D3DRS_ALPHATESTENABLE, on ? TRUE : FALSE);
			device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			break;
		default:
			throw Exception();
	}*/
}

void Graphics::setRenderState(RenderState state, int v) {
	switch (state) {
	case DepthTestCompare:
		switch (v) {
		default:
			case ZCompareAlways      : v = GL_ALWAYS; break;
			case ZCompareNever       : v = GL_NEVER; break;
			case ZCompareEqual       : v = GL_EQUAL; break;
			case ZCompareNotEqual    : v = GL_NOTEQUAL; break;
			case ZCompareLess        : v = GL_LESS; break;
			case ZCompareLessEqual   : v = GL_LEQUAL; break;
			case ZCompareGreater     : v = GL_GREATER; break;
			case ZCompareGreaterEqual: v = GL_GEQUAL; break;
		}
		glDepthFunc(v);
		break;
	case BackfaceCulling:
		switch (v) {
		case Clockwise:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glCheckErrors();
			break;
		case CounterClockwise:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glCheckErrors();
			break;
		case NoCulling:
			glDisable(GL_CULL_FACE);
			glCheckErrors();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	/*switch (state) {
		case DepthTestCompare:
			switch (v) {
					// TODO: Cmp-Konstanten systemabhaengig abgleichen
				default:
				case ZCmp_Always      : v = D3DCMP_ALWAYS; break;
				case ZCmp_Never       : v = D3DCMP_NEVER; break;
				case ZCmp_Equal       : v = D3DCMP_EQUAL; break;
				case ZCmp_NotEqual    : v = D3DCMP_NOTEQUAL; break;
				case ZCmp_Less        : v = D3DCMP_LESS; break;
				case ZCmp_LessEqual   : v = D3DCMP_LESSEQUAL; break;
				case ZCmp_Greater     : v = D3DCMP_GREATER; break;
				case ZCmp_GreaterEqual: v = D3DCMP_GREATEREQUAL; break;
			}
			device->SetRenderState(D3DRS_ZFUNC, v);
			break;
		case FogTypeState:
			switch (v) {
				case LinearFog:
					device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			}
			break;
		case AlphaReferenceState:
			device->SetRenderState(D3DRS_ALPHAREF, (DWORD)v);
			break;
		default:
			throw Exception();
	}*/
}

void Graphics::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
#if defined(SYS_IOS)
	glBindVertexArrayOES(arrayId);
	glCheckErrors();
#elif !defined(SYS_ANDROID) && !defined(SYS_HTML5) && !defined(SYS_TIZEN)
	glBindVertexArray(arrayId);
	glCheckErrors();
#endif

	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += vertexBuffers[i]->_set(offset);
	}
}

void Graphics::setIndexBuffer(IndexBuffer& indexBuffer) {
	indexBuffer._set();
}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	GLenum texDir;
	switch (dir) {
	case U:
		texDir = GL_TEXTURE_WRAP_S;
		break;
	case V:
		texDir = GL_TEXTURE_WRAP_T;
		break;
	}
	switch (addressing) {
	case Clamp:
		glTexParameteri(GL_TEXTURE_2D, texDir, GL_CLAMP_TO_EDGE);
		break;
	case Repeat:
		glTexParameteri(GL_TEXTURE_2D, texDir, GL_REPEAT);
		break;
	case Border:
		//unsupported
		glTexParameteri(GL_TEXTURE_2D, texDir, GL_CLAMP_TO_EDGE);
		break;
	case Mirror:
		//unsupported
		glTexParameteri(GL_TEXTURE_2D, texDir, GL_REPEAT);
		break;
	}
}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	glActiveTexture(GL_TEXTURE0 + texunit.unit);
	switch (filter) {
	case PointFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case LinearFilter:
	case AnisotropicFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
}

namespace {
	void setMinMipFilters(int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		switch (minFilters[unit]) {
		case PointFilter:
			switch (mipFilters[unit]) {
			case NoMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				break;
			case PointMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case LinearMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				break;
			}
			break;
		case LinearFilter:
		case AnisotropicFilter:
			switch (mipFilters[unit]) {
			case NoMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			case PointMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case LinearMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
			}
			break;
		}
	}
}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	minFilters[texunit.unit] = filter;
	setMinMipFilters(texunit.unit);
}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[texunit.unit] = filter;
	setMinMipFilters(texunit.unit);
}

namespace {
	GLenum convert(BlendingOperation operation) {
		switch (operation) {
		case BlendZero:
			return GL_ZERO;
		case BlendOne:
			return GL_ONE;
		case SourceAlpha:
			return GL_SRC_ALPHA;
		case DestinationAlpha:
			return GL_DST_ALPHA;
		case InverseSourceAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case InverseDestinationAlpha:
			return GL_ONE_MINUS_DST_ALPHA;
		default:
			return GL_ONE;
		}
	}
}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	glBlendFunc(convert(source), convert(destination));
}

void Graphics::setRenderTarget(RenderTarget* texture, int num) {
	glBindFramebuffer(GL_FRAMEBUFFER, texture->_framebuffer);
	glViewport(0, 0, texture->texWidth, texture->texHeight);
}

void Graphics::restoreRenderTarget() {
	glBindFramebuffer(GL_FRAMEBUFFER, originalFramebuffer);
	glViewport(0, 0, System::screenWidth(), System::screenHeight());
}

bool Graphics::renderTargetsInvertedY() {
	return true;
}

bool Graphics::nonPow2TexturesSupported() {
	return false;
}

void Graphics::flush() {
	glFlush();
}
