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
