#include "pch.h"
#include "Graphics.h"
#include <limits>

using namespace Kore;

namespace {
	bool antialiasing = false;
	bool window = true;
}

bool Graphics::antialiasing() {
	return ::antialiasing;
}

void Graphics::setAntialiasing(bool antialiasing) {
	::antialiasing = antialiasing;
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
