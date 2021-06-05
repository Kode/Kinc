
#ifndef OPENGL_1_X

#include "Graphics.h"

#include <kinc/graphics5/graphics.h>

#include <limits>

using namespace Kore;

namespace {
	int samples = 1;
}

int Graphics5::antialiasingSamples() {
	return ::samples;
}

void Graphics5::setAntialiasingSamples(int samples) {
	::samples = samples;
}

bool Graphics5::fullscreen = false;

extern "C" void kinc_internal_resize(int window, int width, int height);

void Graphics5::_resize(int window, int width, int height) {
	kinc_internal_resize(window, width, height);
}

int Graphics5::maxBoundTextures() {
	return kinc_g5_max_bound_textures();
}

// void Graphics5::setVertexBuffer(VertexBuffer& vertexBuffer) {
//	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
//	setVertexBuffers(vertexBuffers, 1);
//}

#endif
