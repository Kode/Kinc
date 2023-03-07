#include <stdbool.h>

static bool kinc_internal_opengl_force_16bit_index_buffer = false;
static int kinc_internal_opengl_max_vertex_attribute_arrays = 0;

#include "OpenGL.c.h"
#include "OpenGLWindow.c.h"
#include "ShaderStorageBufferImpl.c.h"
#include "VrInterface.c.h"
#include "indexbuffer.c.h"
#include "pipeline.c.h"
#include "rendertarget.c.h"
#include "shader.c.h"
#include "texture.c.h"
#include "texturearray.c.h"
#include "vertexbuffer.c.h"
