#pragma once

#include <kinc/color.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void kinc_g1_init(int width, int height);
KINC_FUNC void kinc_g1_begin();
KINC_FUNC void kinc_g1_end();
KINC_FUNC void kinc_g1_set_pixel(int x, int y, float red, float green, float blue);
KINC_FUNC int kinc_g1_width();
KINC_FUNC int kinc_g1_height();

#ifdef __cplusplus
}
#endif
