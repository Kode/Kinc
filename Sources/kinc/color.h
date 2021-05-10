#pragma once

#include <kinc/global.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void kinc_color_components(uint32_t color, float *red, float *green, float *blue, float *alpha);

#define KINC_COLOR_BLACK 0xff000000
#define KINC_COLOR_WHITE 0xffffffff
#define KINC_COLOR_RED 0xffff0000
#define KINC_COLOR_BLUE 0xff0000ff
#define KINC_COLOR_GREEN 0xff00ff00
#define KINC_COLOR_MAGENTA 0xffff00ff
#define KINC_COLOR_YELLOW 0xffffff00
#define KINC_COLOR_CYAN 0xff00ffff

#ifdef __cplusplus
}
#endif
