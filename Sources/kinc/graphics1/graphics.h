#pragma once

#include <kinc/color.h>

#ifdef __cplusplus
extern "C" {
#endif

void kinc_g1_init(int width, int height);
void kinc_g1_begin();
void kinc_g1_end();
void kinc_g1_set_pixel(int x, int y, float red, float green, float blue);
int kinc_g1_width();
int kinc_g1_height();

#ifdef __cplusplus
}
#endif
