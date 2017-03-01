#include "pch.h"
#include "Graphics3.h"
#include <limits>

using namespace Kore;

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

#if 0

void Graphics3::setFloat2(ConstantLocation position, vec2 value) {
	setFloat2(position, value.x(), value.y());
}

void Graphics3::setFloat3(ConstantLocation position, vec3 value) {
	setFloat3(position, value.x(), value.y(), value.z());
}

void Graphics3::setFloat4(ConstantLocation position, vec4 value) {
	setFloat4(position, value.x(), value.y(), value.z(), value.w());
}

#endif

void Graphics3::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = { &vertexBuffer };
	setVertexBuffers(vertexBuffers, 1);
}

