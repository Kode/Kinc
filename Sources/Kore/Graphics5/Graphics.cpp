
#ifndef OPENGL_1_X

#include "pch.h"

#include "Graphics.h"

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

extern "C" void Kinc_Internal_Resize(int window, int width, int height);

void Graphics5::_resize(int window, int width, int height) {
	Kinc_Internal_Resize(window, width, height);
}

// void Graphics5::setVertexBuffer(VertexBuffer& vertexBuffer) {
//	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
//	setVertexBuffers(vertexBuffers, 1);
//}

#endif
