#include "pch.h"

#ifdef KORE_ANDROID

#include "ogl.h"

#include <GLContext.h>

extern "C" bool kinc_opengl_internal_nonPow2RenderTargetsSupported() {
	if (ndk_helper::GLContext::GetInstance()->GetGLVersion() >= 3.0)
		return true;
	else
		return false;
}

#endif
