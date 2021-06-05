#include "graphics.h"

#include <kinc/graphics1/graphics.h>
#include <kinc/math/core.h>
#include <kinc/math/matrix.h>

#include <stdint.h>
#include <string.h>

void kinc_g2_init(int screen_width, int screen_height) {
	kinc_g1_init(screen_width, screen_height);
}

void kinc_g2_begin(void) {
	kinc_g1_begin();
}

void kinc_g2_end(void) {
	kinc_g1_end();
}

void kinc_g2_clear(float r, float g, float b) {
	memset(kinc_internal_g1_image, 0, kinc_internal_g1_tex_width * kinc_internal_g1_h * 4);
}

/*void kinc_g2_draw_image(kinc_image_t *img, float x, float y) {
    int xi = (int)kinc_round(x);
    int yi = (int)kinc_round(y);
    uint32_t *data = (uint32_t *)img->data;

    for (int yy = yi; yy < yi + img->height; ++yy) {
        for (int xx = xi; xx < xi + img->width; ++xx) {
            uint32_t pixel = data[(yy - yi) * img->width + (xx - xi)];
            uint32_t alpha = pixel >> 24;
            uint32_t blue = (pixel >> 16) & 0xff;
            uint32_t green = (pixel >> 8) & 0xff;
            uint32_t red = pixel & 0xff;

            float rf = red / 255.0f;
            float gf = green / 255.0f;
            float bf = blue / 255.0f;

            if (alpha == 0) {
                // nothing
            }
            else if (alpha == 255) {
                kinc_g1_set_pixel(xx, yy, rf, gf, bf);
            }
            else {
                float a = alpha / 255.0f;
                uint32_t old = kinc_internal_g1_image[(yy - yi) * kinc_internal_g1_tex_width + (xx - xi)];
                float oldblue = ((old >> 16) & 0xff) / 255.0f;
                float oldgreen = ((old >> 8) & 0xff) / 255.0f;
                float oldred = (old & 0xff) / 255.0f;

                kinc_g1_set_pixel(xx, yy, rf * a + oldred * (1.0f - a), gf * a + oldgreen * (1.0f - a), bf * a + oldblue * (1.0f - a));
            }
        }
    }
}*/

static void draw_pixel(kinc_image_t *img, int frame_x, int frame_y, float u, float v) {
	// int xi = (int)kinc_round(x);
	// int yi = (int)kinc_round(y);
	uint32_t *data = (uint32_t *)img->data;

	int image_x = (int)kinc_round(u * (img->width - 1));
	int image_y = (int)kinc_round(v * (img->height - 1));

	uint32_t pixel = data[image_y * img->width + image_x];
	uint32_t alpha = pixel >> 24;
	uint32_t blue = (pixel >> 16) & 0xff;
	uint32_t green = (pixel >> 8) & 0xff;
	uint32_t red = pixel & 0xff;

	float rf = red / 255.0f;
	float gf = green / 255.0f;
	float bf = blue / 255.0f;

	if (alpha == 0) {
		// nothing
	}
	else if (alpha == 255) {
		kinc_g1_set_pixel(frame_x, frame_y, rf, gf, bf);
	}
	else {
		float a = alpha / 255.0f;
		uint32_t old = kinc_internal_g1_image[frame_y * kinc_internal_g1_tex_width + frame_x];
		float oldblue = ((old >> 16) & 0xff) / 255.0f;
		float oldgreen = ((old >> 8) & 0xff) / 255.0f;
		float oldred = (old & 0xff) / 255.0f;

		kinc_g1_set_pixel(frame_x, frame_y, rf * a + oldred * (1.0f - a), gf * a + oldgreen * (1.0f - a), bf * a + oldblue * (1.0f - a));
	}
}

typedef struct Point2D {
	int x, y;
} Point2D_t;

static int orient2d(Point2D_t a, Point2D_t b, Point2D_t c) {
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static int min4(int a, int b, int c, int d) {
	return kinc_mini(kinc_mini(a, b), kinc_mini(c, d));
}

static int max4(int a, int b, int c, int d) {
	return kinc_maxi(kinc_maxi(a, b), kinc_maxi(c, d));
}

static void drawQuad(kinc_image_t *img, Point2D_t v0, Point2D_t v1, Point2D_t v2, Point2D_t v3) {
	// Compute triangle bounding box
	int minX = min4(v0.x, v1.x, v2.x, v3.x);
	int minY = min4(v0.y, v1.y, v2.y, v3.y);
	int maxX = max4(v0.x, v1.x, v2.x, v3.x);
	int maxY = max4(v0.y, v1.y, v2.y, v3.y);

	// Clip against screen bounds
	minX = kinc_maxi(minX, 0);
	minY = kinc_maxi(minY, 0);
	maxX = kinc_mini(maxX, kinc_internal_g1_w - 1);
	maxY = kinc_mini(maxY, kinc_internal_g1_h - 1);

	// Rasterize
	Point2D_t p;
	for (p.y = minY; p.y <= maxY; p.y++) {
		for (p.x = minX; p.x <= maxX; p.x++) {
			// Determine barycentric coordinates
			int w0 = orient2d(v0, v1, p);
			int w1 = orient2d(v1, v2, p);
			int w2 = orient2d(v2, v3, p);
			int w3 = orient2d(v3, v0, p);

			float u = w0 / (float)(w0 + w2);
			float v = w1 / (float)(w1 + w3);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0 && w3 >= 0) {
				// renderPixel(p, w0, w1, w2);
				// kinc_g1_set_pixel(p.x, p.y, u, v, 0.0f);
				draw_pixel(img, p.x, p.y, u, v);
			}
		}
	}
}

static kinc_matrix3x3_t transform;

void kinc_g2_draw_image(kinc_image_t *img, float x, float y) {
	kinc_vector3_t _0;
	_0.x = x;
	_0.y = y;
	_0.z = 1.0f;
	kinc_vector3_t _1;
	_1.x = x + img->width;
	_1.y = y;
	_1.z = 1.0f;
	kinc_vector3_t _2;
	_2.x = x + img->width;
	_2.y = y + img->height;
	_2.z = 1.0f;
	kinc_vector3_t _3;
	_3.x = x;
	_3.y = y + img->height;
	_3.z = 1.0f;

	_0 = kinc_matrix3x3_multiply_vector(&transform, _0);
	_1 = kinc_matrix3x3_multiply_vector(&transform, _1);
	_2 = kinc_matrix3x3_multiply_vector(&transform, _2);
	_3 = kinc_matrix3x3_multiply_vector(&transform, _3);

	Point2D_t v0, v1, v2, v3;
	v0.x = (int)kinc_round(_0.x);
	v0.y = (int)kinc_round(_0.y);
	v1.x = (int)kinc_round(_1.x);
	v1.y = (int)kinc_round(_1.y);
	v2.x = (int)kinc_round(_2.x);
	v2.y = (int)kinc_round(_2.y);
	v3.x = (int)kinc_round(_3.x);
	v3.y = (int)kinc_round(_3.y);
	drawQuad(img, v0, v1, v2, v3);
}

void kinc_g2_set_rotation(float angle, float centerx, float centery) {
	kinc_matrix3x3_t translation1 = kinc_matrix3x3_translation(centerx, centery);
	kinc_matrix3x3_t rotation = kinc_matrix3x3_rotation_z(angle);
	kinc_matrix3x3_t translation2 = kinc_matrix3x3_translation(-centerx, -centery);
	kinc_matrix3x3_t transformation1 = kinc_matrix3x3_multiply(&translation1, &rotation);
	transform = kinc_matrix3x3_multiply(&transformation1, &translation2);
}