#pragma once

#include <kinc/image.h>

#include "textureunit.h"

#include <Kore/Texture5Impl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_texture {
	int texWidth;
	int texHeight;
	Texture5Impl impl;
} kinc_g5_texture_t;

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format, bool readable);
void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format, bool readable);
void kinc_g5_texture_init_from_file(kinc_g5_texture_t *texture, const char *filename, bool readable);
void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable);
void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable);
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture);
#ifdef KORE_ANDROID
void kinc_g5_texture_init(kinc_g5_texture_t *texture, unsigned texid);
#endif
void kinc_g5_internal_texture_set(kinc_g5_texture_t *texture, int unit);
void kinc_g5_internal_texture_set_image(kinc_g5_texture_t *texture, int unit);
uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture);
void kinc_g5_texture_unlock(kinc_g5_texture_t *texture);
void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);
#ifdef KORE_IOS
void kinc_g5_texture_upload(kinc_g5_texture_t *texture, uint8_t *data);
#endif
void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels);
void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level);

int kinc_g5_texture_stride(kinc_g5_texture_t *texture);

#ifdef __cplusplus
}
#endif
