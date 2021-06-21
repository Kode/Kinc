#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/math/matrix.h>

/*! \file graphics.h
    \brief This is still in progress, please don't use it.
*/

#ifdef __cplusplus
extern "C" {
#endif

void kinc_g2_init(int screen_width, int screen_height);
void kinc_g2_begin(void);
void kinc_g2_end(void);
void kinc_g2_clear(float r, float g, float b);
void kinc_g2_draw_image(kinc_image_t *img, float x, float y);
// void drawScaledSubImage(Graphics4::Texture *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_set_rotation(float angle, float centerx, float centery);

#ifdef __cplusplus
}
#endif
