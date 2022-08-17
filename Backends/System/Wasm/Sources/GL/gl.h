#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int GLint;
typedef unsigned GLuint;
typedef size_t GLsizei;
typedef float GLfloat;
typedef int GLenum;
typedef bool GLboolean;
typedef uint8_t GLubyte;
typedef float GLclampf;
typedef int GLbitfield;
typedef uint64_t GLsizeiptr;
typedef uint64_t GLintptr;
typedef char GLchar;

#define GL_CLAMP_TO_EDGE 0
#define GL_INT_VEC2 1
#define GL_INT_VEC3 2
#define GL_INT_VEC4 3
#define GL_FLOAT_VEC2 4
#define GL_FLOAT_VEC3 5
#define GL_FLOAT_VEC4 6
#define GL_FLOAT_MAT4 7
#define GL_MAJOR_VERSION 8
#define GL_EXTENSIONS 9
#define GL_TRUE 10
#define GL_FALSE 11
#define GL_UNSIGNED_SHORT 12
#define GL_UNSIGNED_INT 13
#define GL_TRIANGLES 14
#define GL_SCISSOR_TEST 15
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 16
#define GL_TEXTURE0 17
#define GL_TEXTURE_WRAP_S 18
#define GL_TEXTURE_WRAP_T 19
#define GL_REPEAT 20
#define GL_DEPTH_TEST 21
#define GL_COLOR_BUFFER_BIT 22
#define GL_DEPTH_BUFFER_BIT 23
#define GL_STENCIL_BUFFER_BIT 24
#define GL_TEXTURE_2D 25
#define GL_TEXTURE_MIN_FILTER 26
#define GL_TEXTURE_MAG_FILTER 27
#define GL_NEAREST 28
#define GL_LINEAR 29
#define GL_NEAREST_MIPMAP_NEAREST 30
#define GL_NEAREST_MIPMAP_LINEAR 31
#define GL_LINEAR_MIPMAP_NEAREST 32
#define GL_LINEAR_MIPMAP_LINEAR 33
#define GL_LEQUAL 34
#define GL_NONE 35
#define GL_TEXTURE_CUBE_MAP 36
#define GL_TEXTURE_MIN_LOD 37
#define GL_TEXTURE_MAX_LOD 38
#define GL_FRAMEBUFFER 39
#define GL_COLOR_ATTACHMENT0 40
#define GL_DEPTH_ATTACHMENT 41
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 42
#define GL_ANY_SAMPLES_PASSED 43
#define GL_QUERY_RESULT_AVAILABLE 44
#define GL_QUERY_RESULT 45
#define GL_FRONT 46
#define GL_BACK 47
#define GL_ALWAYS 48
#define GL_EQUAL 49
#define GL_GREATER 50
#define GL_GEQUAL 51
#define GL_LESS 52
#define GL_NEVER 53
#define GL_NOTEQUAL 54
#define GL_STATIC_DRAW 55
#define GL_DYNAMIC_DRAW 56
#define GL_ELEMENT_ARRAY_BUFFER 57
#define GL_DECR 58
#define GL_DECR_WRAP 59
#define GL_INCR 60
#define GL_INCR_WRAP 61
#define GL_INVERT 62
#define GL_KEEP 63
#define GL_REPLACE 64
#define GL_ZERO 65
#define GL_ONE 66
#define GL_SRC_ALPHA 67
#define GL_DST_ALPHA 68
#define GL_ONE_MINUS_SRC_ALPHA 69
#define GL_ONE_MINUS_DST_ALPHA 70
#define GL_SRC_COLOR 71
#define GL_DST_COLOR 72
#define GL_ONE_MINUS_SRC_COLOR 73
#define GL_ONE_MINUS_DST_COLOR 74
#define GL_FUNC_ADD 75
#define GL_FUNC_SUBTRACT 76
#define GL_FUNC_REVERSE_SUBTRACT 77
#define GL_MIN 78
#define GL_MAX 79
#define GL_VERTEX_SHADER 80
#define GL_FRAGMENT_SHADER 81
#define GL_COMPILE_STATUS 82
#define GL_INFO_LOG_LENGTH 83
#define GL_LINK_STATUS 84
#define GL_STENCIL_TEST 85
#define GL_CULL_FACE 86
#define GL_BLEND 87
#define GL_FLOAT 88
#define GL_ACTIVE_UNIFORMS 89
#define GL_DEPTH24_STENCIL8_OES 90
#define GL_DEPTH_COMPONENT 91
#define GL_DEPTH_COMPONENT16 92
#define GL_STENCIL_ATTACHMENT 93
#define GL_RENDERBUFFER 94
#define GL_RGBA 95
#define GL_LUMINANCE 96
#define GL_UNSIGNED_BYTE 97
#define GL_RGB 98
#define GL_UNPACK_ALIGNMENT 99
#define GL_ARRAY_BUFFER 100
#define GL_BYTE 101
#define GL_SHORT 102
#define GL_INT 103

static void glUniform1i(GLint location, GLint v0) {}
static void glUniform2i(GLint location, GLint v0, GLint v1) {}
static void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {}
static void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {}
static void glUniform1iv(GLint location, GLsizei count, const GLint *value) {}
static void glUniform2iv(GLint location, GLsizei count, const GLint *value) {}
static void glUniform3iv(GLint location, GLsizei count, const GLint *value) {}
static void glUniform4iv(GLint location, GLsizei count, const GLint *value) {}
static void glUniform1f(GLint location, GLfloat v0) {}
static void glUniform2f(GLint location, GLfloat v0, GLfloat v1) {}
static void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {}
static void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {}
static void glUniform1fv(GLint location, GLsizei count, const GLfloat *value) {}
static void glUniform2fv(GLint location, GLsizei count, const GLfloat *value) {}
static void glUniform3fv(GLint location, GLsizei count, const GLfloat *value) {}
static void glUniform4fv(GLint location, GLsizei count, const GLfloat *value) {}
static void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
static void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {}
static void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {}
static void glGetIntegerv(GLenum pname, GLint *data) {}
static const GLubyte *glGetString(GLenum name) { return NULL; }
static void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {}
static void glEnable(GLenum cap) {}
static void glDisable(GLenum cap) {}
static void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {}
static void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {}
static void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {}
static void glDepthMask(GLboolean flag) {}
static void glClearDepthf(GLclampf depth) {}
static void glStencilMask(GLuint mask) {}
static void glClearStencil(GLint s) {}
static void glClear(GLbitfield mask) {}
static void glTexParameteri(GLenum target, GLenum pname, GLint param) {}
static void glActiveTexture(GLenum texture) {}
static void glGetFloatv(GLenum pname, GLfloat *params) {}
static void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {}
static void glBindFramebuffer(GLenum target, GLuint framebuffer) {}
static void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {}
static void glGenQueries(GLsizei n, GLuint *ids) {}
static void glDeleteQueries(GLsizei n, const GLuint *ids) {}
static void glBeginQuery(GLenum target, GLuint id) {}
static void glEndQuery(GLenum target) {}
static void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {}
static void glDrawArrays(GLenum mode, GLint first, GLsizei count) {}
static void glFlush(void) {}
static void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {}
static void glGenBuffers(GLsizei n, GLuint *buffers) {}
static void glDeleteBuffers(GLsizei n, const GLuint *buffers) {}
static void glBindBuffer(GLenum target, GLuint buffer) {}
static void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {}
static GLuint glCreateProgram(void) { return 0; }
static void glDeleteProgram(GLuint program) {}
static GLuint glCreateShader(GLenum shaderType) { return 0; }
static void glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) {}
static void glCompileShader(GLuint shader) {}
static void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {}
static void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {}
static void glAttachShader(GLuint program, GLuint shader) {}
static void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name) {}
static void glLinkProgram(GLuint program) {}
static void glGetProgramiv(GLuint program, GLenum pname, GLint *params) {}
static void glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {}
static void glUseProgram(GLuint program) {}
static void glStencilMaskSeparate(GLenum face, GLuint mask) {}
static void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {}
static void glDepthFunc(GLenum func) {}
static void glCullFace(GLenum mode) {}
static void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {}
static void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {}
static GLint glGetUniformLocation(GLuint program, const GLchar *name) { return 0; }
static void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {}
static void glGenTextures(GLsizei n, GLuint *textures) {}
static void glDeleteTextures(GLsizei n, const GLuint *textures) {}
static void glBindTexture(GLenum target, GLuint texture) {}
static void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data) {}
static void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers) {}
static void glBindRenderbuffer(GLenum target, GLuint renderbuffer) {}
static void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {}
static void glGenFramebuffers(GLsizei n, GLuint *ids) {}
static void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {}
static void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data) {}
static void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {}
static void glGenerateMipmap(GLenum target) {}
static void glDeleteShader(GLuint shader) {}
static void glPixelStorei(GLenum pname, GLint param) {}
static void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) {}
static void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {}
static void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {}
static void glEnableVertexAttribArray(GLuint index) {}
static void glDisableVertexAttribArray(GLuint index) {}
static void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {}
