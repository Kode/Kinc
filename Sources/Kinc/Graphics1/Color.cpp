#include "pch.h"

#include "Color.h"

using namespace Kore;

Graphics1::Color::Color(uint color) {
	getColorFromHex(color, R, G, B, A);
}

void Graphics1::Color::getColorFromHex(uint color, float& red, float& green, float& blue, float& alpha) {
	alpha = ((color & 0xff000000) >> 24) / 255.0f;
	red = ((color & 0x00ff0000) >> 16) / 255.0f;
	green = ((color & 0x0000ff00) >> 8) / 255.0f;
	blue = (color & 0x000000ff) / 255.0f;
}
