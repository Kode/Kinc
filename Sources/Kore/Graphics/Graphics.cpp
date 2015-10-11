#include "pch.h"
#include "Graphics.h"
#include <limits>

using namespace Kore;

namespace {
	int samples = 1;
	bool window = true;
}

int Graphics::antialiasingSamples() {
	return ::samples;
}

void Graphics::setAntialiasingSamples(int samples) {
	::samples = samples;
}

bool Graphics::hasWindow() {
	return ::window;
}

void Graphics::setWindow(bool value) {
	::window = value;
}

bool Kore::Graphics::fullscreen = false;

void Graphics::setFloat2(ConstantLocation position, vec2 value) {
	setFloat2(position, value.x(), value.y());
}

void Graphics::setFloat3(ConstantLocation position, vec3 value) {
	setFloat3(position, value.x(), value.y(), value.z());
}

void Graphics::setFloat4(ConstantLocation position, vec4 value) {
	setFloat4(position, value.x(), value.y(), value.z(), value.w());
}

void Graphics::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = { &vertexBuffer };
	setVertexBuffers(vertexBuffers, 1);
}
