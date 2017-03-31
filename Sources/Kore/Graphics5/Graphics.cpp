
#ifndef OPENGL_1_X

#include "pch.h"

#include "Graphics.h"

#include <limits>

using namespace Kore;

namespace {
	int samples = 1;
	bool window = true;
}

#if !defined(KORE_WINDOWS)
void Graphics5::setup() {}
#endif

int Graphics5::antialiasingSamples() {
	return ::samples;
}

void Graphics5::setAntialiasingSamples(int samples) {
	::samples = samples;
}

bool Graphics5::hasWindow() {
	return ::window;
}

void Graphics5::setWindow(bool value) {
	::window = value;
}

bool Graphics5::fullscreen = false;

void Graphics5::setFloat2(ConstantLocation position, vec2 value) {
	setFloat2(position, value.x(), value.y());
}

void Graphics5::setFloat3(ConstantLocation position, vec3 value) {
	setFloat3(position, value.x(), value.y(), value.z());
}

void Graphics5::setFloat4(ConstantLocation position, vec4 value) {
	setFloat4(position, value.x(), value.y(), value.z(), value.w());
}

void Graphics5::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
	setVertexBuffers(vertexBuffers, 1);
}

#endif
