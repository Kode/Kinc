#pragma once

#include <kinc/global.h>

#include <kinc/graphics4/texture.h>
#include <kinc/math/matrix.h>

/*! \file graphics.h
    \brief This is still in progress, please don't use it.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KINC_KONG

typedef enum kinc_g2_image_scale_quality {
	KINC_G2_IMAGE_SCALE_QUALITY_LOW, // usually point filter
	KINC_G2_IMAGE_SCALE_QUALITY_HIGH // usually bilinear filter
} kinc_g2_image_scale_quality;

// enum HorTextAlignment { TextLeft, TextCenter, TextRight };
// enum VerTextAlignment { TextTop, TextMiddle, TextBottom };

void kinc_g2_init(void);
void kinc_g2_draw_image(kinc_g4_texture_t *img, float x, float y);
void kinc_g2_draw_scaled_sub_image(kinc_g4_texture_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_draw_sub_image(kinc_g4_texture_t *img, float x, float y, float sx, float sy, float sw, float sh);
void kinc_g2_draw_scaled_image(kinc_g4_texture_t *img, float dx, float dy, float dw, float dh);
uint32_t kinc_g2_get_color(void);
void kinc_g2_set_color(uint32_t color);
void kinc_g2_draw_rect(float x, float y, float width, float height, float strength /*= 1.0*/);
void kinc_g2_fill_rect(float x, float y, float width, float height);
void kinc_g2_draw_line(float x1, float y1, float x2, float y2, float strength /*= 1.0*/);
void kinc_g2_fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3);
kinc_g2_image_scale_quality kinc_g2_get_image_scale_quality(void);
void kinc_g2_set_image_scale_quality(kinc_g2_image_scale_quality value);
kinc_g2_image_scale_quality kinc_g2_get_mipmap_scale_quality(void);
void kinc_g2_set_mipmap_scale_quality(kinc_g2_image_scale_quality value);
void kinc_g2_scissor(int x, int y, int width, int height);
void kinc_g2_disable_scissor(void);
void kinc_g2_begin(int width, int height, bool clear /*= true*/, uint32_t clear_color);
void kinc_g2_clear(uint32_t color);
void kinc_g2_flush(void);
void kinc_g2_end(void);
kinc_matrix3x3_t kinc_g2_get_transformation(void);
void kinc_g2_set_transformation(kinc_matrix3x3_t transformation);
void kinc_g2_push_transformation(kinc_matrix3x3_t trans);
kinc_matrix3x3_t kinc_g2_pop_transformation();
void kinc_g2_scale(float x, float y);
void kinc_g2_push_scale(float x, float y);
kinc_matrix3x3_t kinc_g2_translation(float tx, float ty);
void kinc_g2_translate(float tx, float ty);
void kinc_g2_push_translation(float tx, float ty);
kinc_matrix3x3_t kinc_g2_rotation(float angle, float centerx, float centery);
void kinc_g2_rotate(float angle, float centerx, float centery);
void kinc_g2_push_rotation(float angle, float centerx, float centery);
void kinc_g2_push_opacity(float opacity);
float kinc_g2_pop_opacity();
float kinc_g2_get_opacity();
void kinc_g2_set_opacity(float opacity);

#else

void kinc_g2_init(int screen_width, int screen_height);
void kinc_g2_begin(void);
void kinc_g2_end(void);
void kinc_g2_clear(float r, float g, float b);
void kinc_g2_draw_image(kinc_image_t *img, float x, float y);
// void drawScaledSubImage(Graphics4::Texture *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
void kinc_g2_set_rotation(float angle, float centerx, float centery);

#endif

#ifdef __cplusplus
}
#endif
