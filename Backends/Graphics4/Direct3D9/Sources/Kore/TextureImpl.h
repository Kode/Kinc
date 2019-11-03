#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

typedef struct {
	struct IDirect3DTexture9 *texture;
	int stage;
	bool mipmap;
	uint8_t pixfmt;
	int pitch;
} kinc_g4_texture_impl_t;

struct kinc_g4_texture;

void kinc_internal_texture_unmipmap(struct kinc_g4_texture *texture);
void kinc_internal_texture_unset(struct kinc_g4_texture *texture);

#ifdef __cplusplus
}
#endif
