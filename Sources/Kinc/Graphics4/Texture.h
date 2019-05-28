#pragma once

#include <Kinc/Image.h>
#include <Kore/TextureImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef kinc_image_t kinc_g4_image_t;

typedef struct kinc_g4_texture {
	int tex_width;
	int tex_height;
	int tex_depth;
	kinc_image_format_t format;
	Kinc_G4_TextureImpl impl;
} kinc_g4_texture_t;

void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format);
void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format);
void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image);
void kinc_g4_texture_init_from_image3d(kinc_g4_texture_t *texture, kinc_image_t *image);
void kinc_g4_texture_destroy(kinc_g4_texture_t *texture);
#ifdef KORE_ANDROID
void kinc_g4_texture_init_from_id(unsigned texid);
#endif
// void _set(TextureUnit unit);
// void _setImage(TextureUnit unit);
unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture);
void kinc_g4_texture_unlock(kinc_g4_texture_t *texture);
void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);
#if defined(KORE_IOS) || defined(KORE_MACOS)
void kinc_g4_texture_upload(uint8_t *data, int stride);
#endif
void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels);
void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level);

int kinc_g4_texture_stride(kinc_g4_texture_t *texture);

#ifdef __cplusplus
}
#endif
