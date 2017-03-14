#include "pch.h"
#include "Graphics3.h"
#include <limits>

using namespace Kore;

#ifdef OPENGL_1_X

namespace {
	int samples = 1;
	bool window = true;
}

#if !defined(SYS_WINDOWS)
void Graphics3::setup() {
}
#endif

int Graphics3::antialiasingSamples() {
	return ::samples;
}

void Graphics3::setAntialiasingSamples(int samples) {
	::samples = samples;
}

bool Graphics3::hasWindow() {
	return ::window;
}

void Graphics3::setWindow(bool value) {
	::window = value;
}

bool Kore::Graphics3::fullscreen = false;

void Graphics3::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = { &vertexBuffer };
	setVertexBuffers(vertexBuffers, 1);
}

#endif
