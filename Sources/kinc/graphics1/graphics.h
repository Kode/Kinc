#pragma once

#include <kinc/global.h>

#include <kinc/color.h>

#include <assert.h>

/*! \file graphics.h
    \brief Supports a very basic pixel-setting API.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Initializes the G1-API.
/// </summary>
/// <param name="width">The width to be used by the G1-API - typically the window-width</param>
/// <param name="height">The height to be used by the G1-API - typically the window-height</param>
KINC_FUNC void kinc_g1_init(int width, int height);

/// <summary>
/// Typically called once per frame before other G1-functions are called.
/// </summary>
KINC_FUNC void kinc_g1_begin(void);

/// <summary>
/// Typically called once per frame after all G1-drawing is finished. This also swaps the framebuffers
/// so an equivalent call to kinc_g4_swap_buffers is not needed.
/// </summary>
KINC_FUNC void kinc_g1_end(void);

extern uint32_t *kinc_internal_g1_image;
extern int kinc_internal_g1_w, kinc_internal_g1_h, kinc_internal_g1_tex_width;

#if defined(KINC_DYNAMIC_COMPILE) || defined(KINC_DYNAMIC) || defined(KINC_DOCS)

/// <summary>
/// Sets a single pixel to a color.
/// </summary>
/// <param name="x">The x-component of the pixel-coordinate to set</param>
/// <param name="y">The y-component of the pixel-coordinate to set</param>
/// <param name="red">The red-component between 0 and 1</param>
/// <param name="green">The green-component between 0 and 1</param>
/// <param name="blue">The blue-component between 0 and 1</param>
KINC_FUNC void kinc_g1_set_pixel(int x, int y, float red, float green, float blue);

/// <summary>
/// Returns the width used by G1.
/// </summary>
/// <returns>The width</returns>
KINC_FUNC int kinc_g1_width(void);

/// <summary>
/// Returns the height used by G1.
/// </summary>
/// <returns>The height</returns>
KINC_FUNC int kinc_g1_height(void);

#else

// implementation moved to the header to allow easy inlining

static inline void kinc_g1_set_pixel(int x, int y, float red, float green, float blue) {
	assert(x >= 0 && x < kinc_internal_g1_w && y >= 0 && y < kinc_internal_g1_h);
	int r = (int)(red * 255);
	int g = (int)(green * 255);
	int b = (int)(blue * 255);
	kinc_internal_g1_image[y * kinc_internal_g1_tex_width + x] = 0xff << 24 | b << 16 | g << 8 | r;
}

static inline int kinc_g1_width(void) {
	return kinc_internal_g1_w;
}

static inline int kinc_g1_height(void) {
	return kinc_internal_g1_h;
}

#endif

#ifdef __cplusplus
}
#endif
