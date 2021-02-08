#pragma once

#include <kinc/backend/graphics4/texture.h>
#include <kinc/image.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef kinc_image_t kinc_g4_image_t;

typedef struct kinc_g4_texture {
	int tex_width;
	int tex_height;
	int tex_depth;
	kinc_image_format_t format;
	kinc_g4_texture_impl_t impl;
} kinc_g4_texture_t;

KINC_FUNC void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format);
KINC_FUNC void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format);
KINC_FUNC void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image);
KINC_FUNC void kinc_g4_texture_init_from_image3d(kinc_g4_texture_t *texture, kinc_image_t *image);
KINC_FUNC void kinc_g4_texture_destroy(kinc_g4_texture_t *texture);
#ifdef KORE_ANDROID
KINC_FUNC void kinc_g4_texture_init_from_id(kinc_g4_texture_t *texture, unsigned texid);
#endif
// void _set(TextureUnit unit);
// void _setImage(TextureUnit unit);
KINC_FUNC unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture);
KINC_FUNC void kinc_g4_texture_unlock(kinc_g4_texture_t *texture);
KINC_FUNC void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);
#if defined(KORE_IOS) || defined(KORE_MACOS)
KINC_FUNC void kinc_g4_texture_upload(kinc_g4_texture_t *texture, uint8_t *data, int stride);
#endif
KINC_FUNC void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels);
KINC_FUNC void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level);

KINC_FUNC int kinc_g4_texture_stride(kinc_g4_texture_t *texture);

#ifdef __cplusplus
}
#endif
