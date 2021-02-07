#include "pch.h"
#include "OpenGL.h"
#include "VertexBufferImpl.h"
#include <Kore/System.h>
#include <Kore/Math/Core.h>
#include <Kore/Log.h>
#include "ogl.h"
#include <cstdio>
#include <cassert>

#ifdef KORE_IOS
#include <OpenGLES/ES1/glext.h>
#endif

#ifdef KORE_WINDOWS
	#include <GL/wglew.h>

	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>

	#pragma comment(lib, "opengl32.lib")
	#pragma comment(lib, "glu32.lib")
#endif

using namespace Kore;

namespace Kore {
#if !defined(KORE_IOS) && !defined(KORE_ANDROID)
	extern bool programUsesTessellation;
#endif
}

namespace {
#ifdef KORE_WINDOWS
	HINSTANCE instance = 0;
    HDC deviceContexts[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	HGLRC glContexts[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
#endif

	Graphics3::TextureFilter minFilters[10][32];
	Graphics3::MipmapFilter mipFilters[10][32];
	int originalFramebuffer[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	uint arrayId[10];
	
	int _width;
	int _height;
	int _renderTargetWidth;
	int _renderTargetHeight;
	bool renderToBackbuffer;

	bool depthTest = false;
	bool depthMask = false;

    struct wvpTransform_t {
        mat4 projection;
        mat4 view;
        mat4 world;
    } g_wvpTransform;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	void* glesDrawBuffers;
#endif
}

void Graphics3::destroy(int windowId) {
#ifdef KORE_WINDOWS
	if (glContexts[windowId]) {
		if (!wglMakeCurrent(nullptr, nullptr)) {
			//MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(glContexts[windowId])) {
			//MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		glContexts[windowId] = nullptr;
	}

	HWND windowHandle = (HWND)System::windowHandle(windowId);

	// TODO (DK) shouldn't 'deviceContexts[windowId] = nullptr;' be moved out of here?
	if (deviceContexts[windowId] && !ReleaseDC(windowHandle, deviceContexts[windowId])) {
		//MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		deviceContexts[windowId] = nullptr;
	}
#endif

	System::destroyWindow(windowId);
}

#undef CreateWindow

#ifdef KORE_WINDOWS
namespace Kore { namespace System {
	extern int currentDeviceId;
}}
#endif

#ifdef KORE_WINDOWS
void Graphics3::setup() {
}
#endif

void Graphics3::init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
#ifdef KORE_WINDOWS
	HWND windowHandle = (HWND)System::windowHandle(windowId);

#ifndef VR_RIFT
	// TODO (DK) use provided settings for depth/stencil buffer

	PIXELFORMATDESCRIPTOR pfd =			        // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),	        // Size Of This Pixel Format Descriptor
		1,								        // Version Number
		PFD_DRAW_TO_WINDOW |			        // Format Must Support Window
		PFD_SUPPORT_OPENGL |			        // Format Must Support OpenGL
		PFD_DOUBLEBUFFER,				        // Must Support Double Buffering
		PFD_TYPE_RGBA,					        // Request An RGBA Format
		32,								        // Select Our Color Depth
		0, 0, 0, 0, 0, 0,				        // Color Bits Ignored
		0,								        // No Alpha Buffer
		0,								        // Shift Bit Ignored
		0,								        // No Accumulation Buffer
		0, 0, 0, 0,						        // Accumulation Bits Ignored
		static_cast<BYTE>(depthBufferBits),		// 16Bit Z-Buffer (Depth Buffer)
		static_cast<BYTE>(stencilBufferBits),	// 8Bit Stencil Buffer
		0,								        // No Auxiliary Buffer
		PFD_MAIN_PLANE,					        // Main Drawing Layer
		0,								        // Reserved
		0, 0, 0							        // Layer Masks Ignored
	};

	deviceContexts[windowId] = GetDC(windowHandle);
	GLuint pixelFormat = ChoosePixelFormat(deviceContexts[windowId], &pfd);
	SetPixelFormat(deviceContexts[windowId], pixelFormat, &pfd);
	HGLRC tempGlContext = wglCreateContext(deviceContexts[windowId]);
	wglMakeCurrent(deviceContexts[windowId], tempGlContext);
	Kore::System::currentDeviceId = windowId;

	// TODO (DK) make a Graphics3::setup() (called from System::setup()) and call it there only once?
	if (windowId == 0) {
		glewInit();
	}

    #if 0
	if (wglewIsSupported("WGL_ARB_create_context") == 1) {
		int attributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		glContexts[windowId] = wglCreateContextAttribsARB(deviceContexts[windowId], glContexts[0], attributes);
		glCheckErrors();
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tempGlContext);
		wglMakeCurrent(deviceContexts[windowId], glContexts[windowId]);
		glCheckErrors();
	}
	else
    #endif
    {
		glContexts[windowId] = tempGlContext;
	}

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle); // Slightly Higher Priority
	SetFocus(windowHandle); // Sets Keyboard Focus To The Window
#else
	deviceContexts[windowId] = GetDC(windowHandle);
	glContexts[windowId] = wglGetCurrentContext();
	glewInit();
#endif
#endif

#ifndef VR_RIFT
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setRenderState(DepthTest, false);
	glViewport(0, 0, System::windowWidth(windowId), System::windowHeight(windowId));
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFramebuffer[windowId]);

	for (int i = 0; i < 32; ++i) {
		minFilters[windowId][i] = LinearFilter;
		mipFilters[windowId][i] = NoMipFilter;
	}
#endif

#ifdef KORE_WINDOWS
	if (windowId == 0) {
		// TODO (DK) check if we actually want vsync
		if (wglSwapIntervalEXT != nullptr) wglSwapIntervalEXT(1);
	}
#endif

#if defined(KORE_IOS)
	glGenVertexArraysOES(1, &arrayId[windowId]);
#elif defined(KORE_MACOS)
    glGenVertexArraysAPPLE(1, &arrayId[windowId]);
#elif !defined(KORE_ANDROID) && !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_PI)
	glGenVertexArrays(1, &arrayId[windowId]);
#endif
    glCheckErrors();
	
	_width = System::windowWidth(0);
	_height = System::windowHeight(0);
	_renderTargetWidth = _width;
	_renderTargetHeight = _height;
	renderToBackbuffer = true;

#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
	glesDrawBuffers = (void*)eglGetProcAddress("glDrawBuffers");
#endif
}

void Graphics3::changeResolution(int width, int height) {
	_width = width;
	_height = height;
	if (renderToBackbuffer) {
		_renderTargetWidth = _width;
		_renderTargetHeight = _height;
	}
}

// TODO (DK) should return displays refreshrate?
unsigned Graphics3::refreshRate() {
	return 60;
}

// TODO (DK) should check the extension and return wheter it's enabled (wglSwapIntervalEXT(1)) or not?
bool Graphics3::vsynced() {
	return true;
}

static void invertMatrixEntry(mat4& m, int row, int col)
{
    m.Set(row, col, -m.get(row, col));
}

// Invert matrix Z-axis (for view matrices)
static void invertMatrixZ(mat4& m)
{
    invertMatrixEntry(m, 2, 0);
    invertMatrixEntry(m, 2, 1);
    invertMatrixEntry(m, 2, 2);
    invertMatrixEntry(m, 2, 3);
}

/*
Converts the specified left-handed projection matrix
into a right-handed projection matrix (required for OpenGL).
*/
static void convertToRightHandedProjection(mat4& m) {
    // Invert Z-axis of projection matrix (along 3rd column, i.e. zero-based-index 2)
    invertMatrixEntry(m, 0, 2);
    invertMatrixEntry(m, 1, 2);
    invertMatrixEntry(m, 2, 2);
    invertMatrixEntry(m, 3, 2);
}

static void uploadModelViewMatrix() {
    // Compute: modelViewMatrix = viewMatrix * worldMatrix
    mat4 modelViewMatrix = g_wvpTransform.view;
    modelViewMatrix *= g_wvpTransform.world;

    // Update GL model-view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelViewMatrix.data);
}

static void uploadProjectionMatrix() {
    // Update GL projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(g_wvpTransform.projection.data);
}

void Graphics3::setFogColor(const Graphics1::Color& color) {
    glFogfv(GL_FOG_COLOR, &(color.R));
}

void Graphics3::setViewMatrix(const mat4& value) {
    // Convert view matrix from left-handed to right-handed coordinate system
    g_wvpTransform.view = value;
    #ifndef G3_DISABLE_AUTO_PROJECTION
    invertMatrixZ(g_wvpTransform.view);
    #endif
    uploadModelViewMatrix();
}

void Graphics3::setWorldMatrix(const mat4& value) {
    g_wvpTransform.world = value;
    uploadModelViewMatrix();
}

void Graphics3::setProjectionMatrix(const mat4& value) {
    g_wvpTransform.projection = value;
    #ifndef G3_DISABLE_AUTO_PROJECTION
    convertToRightHandedProjection(g_wvpTransform.projection);
    #endif
    uploadProjectionMatrix();
}

void Graphics3::drawIndexedVertices() {
	drawIndexedVertices(0, IndexBufferImpl::current->count());
}

void Graphics3::drawIndexedVertices(int start, int count) {
#ifdef KORE_OPENGL_ES
#if defined(KORE_ANDROID) || defined(KORE_PI)
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void*)(start * sizeof(GL_UNSIGNED_SHORT)));
#else
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
#endif
    glCheckErrors();
#else
    {
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(start * sizeof(GL_UNSIGNED_INT)));
		glCheckErrors();
	}
#endif
}

void Graphics3::swapBuffers(int contextId) {
#ifdef KORE_WINDOWS
	::SwapBuffers(deviceContexts[contextId]);
#else
	System::swapBuffers(contextId);
#endif
}

#ifdef KORE_IOS
void beginGL();
#endif

#if defined(KORE_WINDOWS)
void Graphics3::makeCurrent(int contextId) {
	wglMakeCurrent(deviceContexts[contextId], glContexts[contextId]);
}
#endif

void Graphics3::begin(int contextId) {
	if (System::currentDevice() != -1) {
		if (System::currentDevice() != contextId) {
			log(Warning, "begin: wrong glContext is active");
		}
		else {
			//**log(Warning, "begin: a glContext is still active");
		}

		//return; // TODO (DK) return here?
	}

	//System::setCurrentDevice(contextId);
    System::makeCurrent(contextId);

	glViewport(0, 0, _width, _height);
	
#ifdef KORE_IOS
	beginGL();
#endif

#ifdef KORE_ANDROID
	// if rendered to a texture, strange things happen if the backbuffer is not cleared
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

void Graphics3::viewport(int x, int y, int width, int height) {
	glViewport(x, _renderTargetHeight - y - height, width, height);
}

void Graphics3::scissor(int x, int y, int width, int height) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, _renderTargetHeight - y - height, width, height);
}

void Graphics3::disableScissor() {
	glDisable(GL_SCISSOR_TEST);
}

namespace {
	GLenum convert(Graphics3::StencilAction action) {
		switch (action) {
		case Graphics3::Decrement:
			return GL_DECR;
		case Graphics3::DecrementWrap:
			return GL_DECR_WRAP;
		case Graphics3::Increment:
			return GL_INCR;
		case Graphics3::IncrementWrap:
			return GL_INCR_WRAP;
		case Graphics3::Invert:
			return GL_INVERT;
		case Graphics3::Keep:
			return GL_KEEP;
		case Graphics3::Replace:
			return GL_REPLACE;
		case Graphics3::Zero:
			return GL_ZERO;
		}
        return 0;
	}
}

void Graphics3::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {
	if (compareMode == ZCompareAlways && bothPass == Keep
	&& depthFail == Keep && stencilFail == Keep) {
		glDisable(GL_STENCIL_TEST);
	}
	else {
		glEnable(GL_STENCIL_TEST);
		int stencilFunc = 0;
		switch (compareMode) {
		case ZCompareAlways:
			stencilFunc = GL_ALWAYS;
			break;
		case ZCompareEqual:
			stencilFunc = GL_EQUAL;
			break;
		case ZCompareGreater:
			stencilFunc = GL_GREATER;
			break;
		case ZCompareGreaterEqual:
			stencilFunc = GL_GEQUAL;
			break;
		case ZCompareLess:
			stencilFunc = GL_LESS;
			break;
		case ZCompareLessEqual:
			stencilFunc = GL_LEQUAL;
			break;
		case ZCompareNever:
			stencilFunc = GL_NEVER;
			break;
		case ZCompareNotEqual:
			stencilFunc = GL_NOTEQUAL;
			break;
		}
		glStencilMask(writeMask);
		glStencilOp(convert(stencilFail), convert(depthFail), convert(bothPass));
		glStencilFunc(stencilFunc, referenceValue, readMask);
	}
}

/*void glCheckErrors() {
	if (System::currentDevice() == -1) {
		log(Warning, "no OpenGL device context is set");
		return;
	}

//#ifdef _DEBUG
	GLenum code = glGetError();
	while (code != GL_NO_ERROR) {
		//std::printf("GLError: %s\n", glewGetErrorString(code));
		switch (code) {
		case GL_INVALID_VALUE:
			log(Warning, "OpenGL: Invalid value");
			break;
		case GL_INVALID_OPERATION:
			log(Warning, "OpenGL: Invalid operation");
			break;
		default:
			log(Warning, "OpenGL: Error code %i", code);
			break;
		}
		code = glGetError();
	}
//#endif
}*/

#ifdef KORE_WINDOWS
void Graphics3::clearCurrent() {
	wglMakeCurrent(nullptr, nullptr);
}
#endif

// TODO (DK) this never gets called on some targets, needs investigation?
void Graphics3::end(int windowId) {
	//glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	glCheckErrors();

	if (System::currentDevice() == -1) {
		log(Warning, "end: a glContext wasn't active");
	}

	if (System::currentDevice() != windowId) {
		log(Warning, "end: wrong glContext is active");
	}

	System::clearCurrent();
}

void Graphics3::clear(uint flags, uint color, float depth, int stencil) {
	glClearColor(((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f, (color & 0xff000000) / 255.0f);
	glCheckErrors();
	if (flags & ClearDepthFlag) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glCheckErrors();
	}
#ifdef KORE_OPENGL_ES
	glClearDepthf(depth);
#else
	glClearDepth(depth);
#endif
    glCheckErrors();
	glStencilMask(0xff);
	glCheckErrors();
	glClearStencil(stencil);
	glCheckErrors();
	GLbitfield oglflags =
		  ((flags & ClearColorFlag) ? GL_COLOR_BUFFER_BIT : 0)
		| ((flags & ClearDepthFlag) ? GL_DEPTH_BUFFER_BIT : 0)
		| ((flags & ClearStencilFlag) ? GL_STENCIL_BUFFER_BIT : 0);
	glClear(oglflags);
	glCheckErrors();
	if (depthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
	glCheckErrors();
	if (depthMask) {
		glDepthMask(GL_TRUE);
	}
	else {
		glDepthMask(GL_FALSE);
	}
	glCheckErrors();
}

void Graphics3::setColorMask(bool red, bool green, bool blue, bool alpha) {
	glColorMask(red, green, blue, alpha);
}

void Graphics3::setMaterialState(MaterialState state, const vec4& value) {
    switch (state) {
    case AmbientColor:
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, value.values);
        break;
    case DiffuseColor:
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value.values);
        break;
    case SpecularColor:
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, value.values);
        break;
    case EmissionColor:
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, value.values);
        break;
    case SolidColor:
        glColor4fv(value.values);
        break;
    default:
        break;
    }
}

void Graphics3::setMaterialState(MaterialState state, float value) {
    switch (state) {
    case ShininessExponent:
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, max(0.0f, min(value, 180.0f)));
        break;
    default:
        break;
    }
}

void Graphics3::setTextureMapping(TextureUnit texunit, TextureMapping mapping, bool on) {
	glActiveTexture(GL_TEXTURE0 + texunit.unit);
    switch (mapping) {
    case Texture1D:
        // Enable/disabled 1D texture mapping for active texture layer
        if (on) glEnable(GL_TEXTURE_1D);
        else glDisable(GL_TEXTURE_1D);
        break;
    case Texture2D:
        // Enable/disabled 2D texture mapping for active texture layer
        if (on) glEnable(GL_TEXTURE_2D);
        else glDisable(GL_TEXTURE_2D);
        break;
    case Texture3D:
        // Enable/disabled 3D texture mapping for active texture layer
        if (on) glEnable(GL_TEXTURE_3D);
        else glDisable(GL_TEXTURE_3D);
        break;
    case TextureCubeMap:
        // Enable/disabled cube texture mapping for active texture layer
        if (on) glEnable(GL_TEXTURE_CUBE_MAP);
        else glDisable(GL_TEXTURE_CUBE_MAP);
        break;
    }
}

static GLenum texCoordToGLenum(Graphics3::TextureCoordinate texcoord)
{
    switch (texcoord) {
    case Graphics3::TexCoordX: return GL_S;
    case Graphics3::TexCoordY: return GL_T;
    case Graphics3::TexCoordZ: return GL_R;
    case Graphics3::TexCoordW: return GL_Q;
    }
    return 0;
}

static GLenum texGenCoordToGLenum(Graphics3::TextureCoordinate texcoord)
{
    switch (texcoord) {
    case Graphics3::TexCoordX: return GL_TEXTURE_GEN_S;
    case Graphics3::TexCoordY: return GL_TEXTURE_GEN_T;
    case Graphics3::TexCoordZ: return GL_TEXTURE_GEN_R;
    case Graphics3::TexCoordW: return GL_TEXTURE_GEN_Q;
    }
    return 0;
}

void Graphics3::setTexCoordGeneration(TextureUnit texunit, TextureCoordinate texcoord, TexCoordGeneration generation) {
	glActiveTexture(GL_TEXTURE0 + texunit.unit);
    switch (generation) {
    case TexGenDisabled:
        // Disable texture coordinate generation for 'texcoord'
        glDisable(texGenCoordToGLenum(texcoord));
        break;
    case TexGenObjectLinear:
        // Enable and configure texture coordinate generation for 'texcoord' with 'generation' mode
        glEnable(texGenCoordToGLenum(texcoord));
        glTexGeni(texCoordToGLenum(texcoord), GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        break;
    case TexGenViewLinear:
        // Enable and configure texture coordinate generation for 'texcoord' with 'generation' mode
        glEnable(texGenCoordToGLenum(texcoord));
        glTexGeni(texCoordToGLenum(texcoord), GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        break;
    case TexGenSphereMap:
        // Enable and configure texture coordinate generation for 'texcoord' with 'generation' mode
        glEnable(texGenCoordToGLenum(texcoord));
        glTexGeni(texCoordToGLenum(texcoord), GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        break;
    case TexGenNormalMap:
        // Enable and configure texture coordinate generation for 'texcoord' with 'generation' mode
        glEnable(texGenCoordToGLenum(texcoord));
        glTexGeni(texCoordToGLenum(texcoord), GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
        break;
    case TexGenReflectionMap:
        // Enable and configure texture coordinate generation for 'texcoord' with 'generation' mode
        glEnable(texGenCoordToGLenum(texcoord));
        glTexGeni(texCoordToGLenum(texcoord), GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
        break;
    }
}

void Graphics3::setRenderState(RenderState state, bool on) {
	switch (state) {
	case DepthWrite:
		if (on) glDepthMask(GL_TRUE);
		else glDepthMask(GL_FALSE);
		depthMask = on;
		break;
	case DepthTest:
		if (on) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
		depthTest = on;
		break;
	case BlendingState:
		if (on) glEnable(GL_BLEND);
		else glDisable(GL_BLEND);
		break;
    case Lighting:
        // Enable/Disable lighting 
        if (on) glEnable(GL_LIGHTING);
        else glDisable(GL_LIGHTING);
        break;
    case Normalize:
        // Enable/disable automatic normalize of normal vectors for non-uniform scaled models
        if (on) glEnable(GL_NORMALIZE);
        else glDisable(GL_NORMALIZE);
        break;
    case FogState:
        // Enable/disable fog
        if (on) glEnable(GL_FOG);
        else glDisable(GL_FOG);
        break;
	default:
		break;
	}

	glCheckErrors();

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

void Graphics3::setRenderState(RenderState state, int v) {
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
		glCheckErrors();
		break;
	case BackfaceCulling:
		switch (v) {
		case Clockwise:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glCheckErrors();
			break;
		case CounterClockwise:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
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
    case FogType:
        switch (v) {
        case LinearFog:
            glFogi(GL_FOG_MODE, GL_LINEAR);
            break;
        case ExpFog:
            glFogi(GL_FOG_MODE, GL_EXP);
            break;
        case Exp2Fog:
            glFogi(GL_FOG_MODE, GL_EXP2);
            break;
        }
        glCheckErrors();
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

void Graphics3::setRenderState(RenderState state, float value) {
	switch (state) {
	case FogStart:
        glFogf(GL_FOG_START, value);
        glCheckErrors();
        break;
	case FogEnd:
        glFogf(GL_FOG_END, value);
        glCheckErrors();
        break;
	case FogDensity:
        glFogf(GL_FOG_DENSITY, value);
        glCheckErrors();
        break;
	default:
		break;
	}
}

// "vertex arrays" are not supported in OpenGL 1.x
// -> "glBindVertexArray" replaced with "glBindBuffer" and several calls to "gl[...]Pointer" 
// -> see VertexBufferImpl::setVertexAttributes
void Graphics3::setVertexBuffers(VertexBuffer** vertexBuffers, int count) {
	int offset = 0;
	for (int i = 0; i < count; ++i) {
		offset += vertexBuffers[i]->_set(offset);
	}
}

void Graphics3::setIndexBuffer(IndexBuffer& indexBuffer) {
	indexBuffer._set();
}

void Graphics3::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}

void Graphics3::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
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
	glCheckErrors();
}

void Graphics3::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	glActiveTexture(GL_TEXTURE0 + texunit.unit);
	glCheckErrors();
	switch (filter) {
	case PointFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case LinearFilter:
	case AnisotropicFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
	glCheckErrors();
}

namespace {
	void setMinMipFilters(int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
		glCheckErrors();
		switch (minFilters[System::currentDevice()][unit]) {
		case Graphics3::PointFilter:
			switch (mipFilters[System::currentDevice()][unit]) {
			case Graphics3::NoMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				break;
			case Graphics3::PointMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case Graphics3::LinearMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				break;
			}
			break;
		case Graphics3::LinearFilter:
		case Graphics3::AnisotropicFilter:
			switch (mipFilters[System::currentDevice()][unit]) {
			case Graphics3::NoMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				break;
			case Graphics3::PointMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case Graphics3::LinearMipFilter:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
			}
			break;
		}
		glCheckErrors();
	}
}

void Graphics3::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	minFilters[System::currentDevice()][texunit.unit] = filter;
	setMinMipFilters(texunit.unit);
}

void Graphics3::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	mipFilters[System::currentDevice()][texunit.unit] = filter;
	setMinMipFilters(texunit.unit);
}

namespace {
	GLenum convert(Graphics3::BlendingOperation operation) {
		switch (operation) {
		case Graphics3::BlendZero:
			return GL_ZERO;
		case Graphics3::BlendOne:
			return GL_ONE;
		case Graphics3::SourceAlpha:
			return GL_SRC_ALPHA;
		case Graphics3::DestinationAlpha:
			return GL_DST_ALPHA;
		case Graphics3::InverseSourceAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case Graphics3::InverseDestinationAlpha:
			return GL_ONE_MINUS_DST_ALPHA;
		case Graphics3::SourceColor:
			return GL_SRC_COLOR;
		case Graphics3::DestinationColor:
			return GL_DST_COLOR;
		case Graphics3::InverseSourceColor:
			return GL_ONE_MINUS_SRC_COLOR;
		case Graphics3::InverseDestinationColor:
			return GL_ONE_MINUS_DST_COLOR;
		default:
			return GL_ONE;
		}
	}
}

void Graphics3::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Graphics3::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	glBlendFunc(convert(source), convert(destination));
	glCheckErrors();
}

void Graphics3::setRenderTarget(RenderTarget* texture, int num, int additionalTargets) {
	if (num == 0) {
		// TODO (DK) uneccessary?
		//System::makeCurrent(texture->contextId);
		glBindFramebuffer(GL_FRAMEBUFFER, texture->_framebuffer);
		glCheckErrors();
		glViewport(0, 0, texture->width, texture->height);
		_renderTargetWidth = texture->width;
		_renderTargetHeight = texture->height;
		renderToBackbuffer = false;
		glCheckErrors();
	}
	
	if (additionalTargets > 0) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + num, GL_TEXTURE_2D, texture->_texture, 0);
		if (num == additionalTargets) {
			GLenum buffers[16];
			for (int i = 0; i <= additionalTargets; ++i) buffers[i] = GL_COLOR_ATTACHMENT0 + i;
#if defined(KORE_OPENGL_ES) && defined(KORE_ANDROID) && KORE_ANDROID_API >= 18
            ((void(*)(GLsizei, GLenum*))glesDrawBuffers)(additionalTargets + 1, buffers);
#elif !defined(KORE_OPENGL_ES)
			glDrawBuffers(additionalTargets + 1, buffers);
#endif
		}
	}
}

void Graphics3::restoreRenderTarget() {
	glBindFramebuffer(GL_FRAMEBUFFER, originalFramebuffer[System::currentDevice()]);
	glCheckErrors();
	int w = System::windowWidth(System::currentDevice());
	int h = System::windowHeight(System::currentDevice());
	glViewport(0, 0, w, h);
	_renderTargetWidth = w;
	_renderTargetHeight = h;
	renderToBackbuffer = true;
	glCheckErrors();
}

void Graphics3::setLight(Light* light, int num) {
    if (light) {
        light->_set(num);
    }
	else {
        glDisable(GL_LIGHT0 + num);
    }
}

bool Graphics3::renderTargetsInvertedY() {
	return true;
}

bool Graphics3::nonPow2TexturesSupported() {
	return true;
}

void Graphics3::flush() {
	glFlush();
	glCheckErrors();
}
