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
	kinc_image_format_t format;
	Texture5Impl impl;
} kinc_g5_texture_t;

KINC_FUNC void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
KINC_FUNC void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format);
KINC_FUNC void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image);
// void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable);
// void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable);
KINC_FUNC void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format);
KINC_FUNC void kinc_g5_texture_destroy(kinc_g5_texture_t *texture);
#ifdef KORE_ANDROID
KINC_FUNC void kinc_g5_texture_init_from_id(kinc_g5_texture_t *texture, unsigned texid);
#endif
void kinc_g5_internal_texture_set(kinc_g5_texture_t *texture, int unit);
void kinc_g5_internal_texture_set_image(kinc_g5_texture_t *texture, int unit);
KINC_FUNC uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture);
KINC_FUNC void kinc_g5_texture_unlock(kinc_g5_texture_t *texture);
KINC_FUNC void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color);
#ifdef KORE_IOS
KINC_FUNC void kinc_g5_texture_upload(kinc_g5_texture_t *texture, uint8_t *data);
#endif
KINC_FUNC void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels);
KINC_FUNC void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level);

KINC_FUNC int kinc_g5_texture_stride(kinc_g5_texture_t *texture);

#ifdef __cplusplus
}
#endif
