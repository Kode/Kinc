#pragma once

#ifdef SYS_WINDOWS
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef SYS_OSX
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#ifdef SYS_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#define OPENGLES
#endif

#ifdef SYS_ANDROID
#include <EGL/egl.h>
#if SYS_ANDROID_API >= 18
#include <GLES3/gl3.h>
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define OPENGLES
#endif

#ifdef SYS_HTML5
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#define OPENGLES
#endif

#ifdef SYS_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#endif

#ifdef SYS_PI
//#define GL_GLEXT_PROTOTYPES
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#define OPENGLES
#endif

#ifdef SYS_TIZEN
#include <gl2.h>
#define OPENGLES
#endif

#include <Kore/Log.h>

#if defined(NDEBUG) || defined(SYS_OSX) || defined(SYS_IOS) || defined(SYS_ANDROID)
#define glCheckErrors() { }
#else
#define glCheckErrors() { GLenum code = glGetError(); while (code != GL_NO_ERROR) { Kore::log(Kore::Error, "GL Error %d %s %d\n", code, __FILE__, __LINE__); } }
#endif
