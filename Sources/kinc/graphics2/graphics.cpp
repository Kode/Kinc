#include "pch.h"

#include "graphics.h"

#include <Kore/Graphics2/Graphics.h>

static Kore::Graphics2::Graphics2 *kinc_kore_g2;

static kinc_matrix3x3_t transformation;

static Kore::Graphics4::Texture *tex;

void kinc_g2_init(void) {
	tex = new Kore::Graphics4::Texture(32, 32, Kore::Graphics1::Image::RGBA32);
	kinc_kore_g2 = new Kore::Graphics2::Graphics2(1024, 768);
}

void kinc_g2_begin(void) {
	kinc_kore_g2->begin();
}

void kinc_g2_end(void) {
	kinc_kore_g2->end();
}

void kinc_g2_draw_image(kinc_g4_texture_t *img, float x, float y) {
	tex->kincTexture = *img;
	tex->width = img->tex_width;
	tex->height = img->tex_height;
	tex->texWidth = img->tex_width;
	tex->texHeight = img->tex_height;
	kinc_kore_g2->drawImage(tex, x, y);
}

void kinc_g2_set_rotation(float angle, float centerx, float centery) {
	kinc_kore_g2->transformation = (Kore::mat3::Translation(centerx, centery) * Kore::mat3::RotationZ(angle)) * Kore::mat3::Translation(-centerx, -centery);
}
