#include "pch.h"

#include "Color.h"

using namespace Kore;

Color::Color(uint color) {
    getColorFromHex(color, R, G, B, A);
}

void Color::getColorFromHex(uint color, float &red, float &green, float &blue, float &alpha) {
    red   = (color & 0x00ff0000) >> 16;
    green = (color & 0x0000ff00) >> 8;
    blue  = (color & 0x000000ff);
    alpha = (color & 0xff000000);
}
