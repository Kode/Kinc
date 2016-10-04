#include "pch.h"

#include "Color.h"

using namespace Kore;

Color::Color(uint color) {
    getColorFromValue(color, R, G, B, A);
}

void Color::getColorFromValue(uint color, float &red, float &green, float &blue, float &alpha) {
    
    red   = ((color & 0x00ff0000) >> 16) / 255.0f;
    green = ((color & 0x0000ff00) >> 8) / 255.0f;
    blue  =  (color & 0x000000ff) / 255.0f;
    alpha =  (color & 0xff000000) / 255.0f;
    
    /*float r = (color & 0x00ff0000) >> 24;
    float g = (color & 0x0000ff00) >> 16;
    float b = (color & 0x000000ff) >> 8;
    float a = (color & 0xff000000)*/
}
