#pragma once

#ifdef KORE_WINDOWS
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef KORE_MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif

#ifdef KORE_IOS
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#endif

#ifdef KORE_ANDROID
#include <EGL/egl.h>
#if KORE_ANDROID_API >= 18
#include <GLES3/gl3.h>
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef KORE_HTML5
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#ifdef KORE_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#endif

#ifdef KORE_PI
//#define GL_GLEXT_PROTOTYPES
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

#ifdef KORE_TIZEN
#include <gl2.h>
#endif

#include <Kore/Log.h>

#if defined(NDEBUG) || defined(KORE_OSX) || defined(KORE_IOS) || defined(KORE_ANDROID) || 1 // Calling glGetError too early means trouble
#define glCheckErrors() { }
#else
#define glCheckErrors() { GLenum code = glGetError(); while (code != GL_NO_ERROR) { Kore::log(Kore::Error, "GL Error %d %s %d\n", code, __FILE__, __LINE__); } }
#endif

#define glCheckErrors2() { GLenum code = glGetError(); while (code != GL_NO_ERROR) { Kore::log(Kore::Error, "GL Error %d %s %d\n", code, __FILE__, __LINE__); } }
