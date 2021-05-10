#pragma once

#include <kinc/global.h>

#include <kinc/color.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void kinc_g1_init(int width, int height);
KINC_FUNC void kinc_g1_begin(void);
KINC_FUNC void kinc_g1_end(void);

#if defined(KINC_DYNAMIC_COMPILE) || defined(KINC_DYNAMIC)

KINC_FUNC void kinc_g1_set_pixel(int x, int y, float red, float green, float blue);
KINC_FUNC int kinc_g1_width(void);
KINC_FUNC int kinc_g1_height(void);

#else

// implementation moved to the header to allow easy inlining

extern int *kinc_internal_g1_image;
extern int kinc_internal_g1_w, kinc_internal_g1_h, kinc_internal_g1_tex_width;

inline void kinc_g1_set_pixel(int x, int y, float red, float green, float blue) {
	if (x < 0 || x >= kinc_internal_g1_w || y < 0 || y >= kinc_internal_g1_h) return;
	int r = (int)(red * 255);
	int g = (int)(green * 255);
	int b = (int)(blue * 255);
	kinc_internal_g1_image[y * kinc_internal_g1_tex_width + x] = 0xff << 24 | b << 16 | g << 8 | r;
}

inline int kinc_g1_width() {
	return kinc_internal_g1_w;
}

inline int kinc_g1_height() {
	return kinc_internal_g1_h;
}

#endif

#ifdef __cplusplus
}
#endif
