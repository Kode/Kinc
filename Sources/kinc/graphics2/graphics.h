#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/math/matrix.h>

#ifdef __cplusplus
extern "C" {
#endif

// void pushRotation(float angle, float centerx, float centery);

void kinc_g2_init(void);
void kinc_g2_begin(void);
void kinc_g2_end(void);
void kinc_g2_draw_image(kinc_g4_texture_t *img, float x, float y);
// void drawScaledSubImage(Graphics4::Texture *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

#ifdef __cplusplus
}
#endif
