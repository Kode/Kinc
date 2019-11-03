#include "pch.h"

#include <kinc/graphics4/texture.h>
#include <kinc/io/filereader.h>

#include <Kore/SystemMicrosoft.h>

#include "Direct3D9.h"

namespace {
	kinc_g4_texture_t *setTextures[16] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                                      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

	D3DFORMAT convert(kinc_image_format_t format) {
		switch (format) {
		case KINC_IMAGE_FORMAT_RGBA32:
		default:
			return D3DFMT_A8R8G8B8;
		case KINC_IMAGE_FORMAT_GREY8:
			return D3DFMT_L8;
		}
	}
}

void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image) {
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	DWORD usage = 0;
	texture->tex_width = image->width;
	texture->tex_height = image->height;
	usage = D3DUSAGE_DYNAMIC;
	kinc_microsoft_affirm_message(
	    device->CreateTexture(image->width, image->height, 1, usage, convert(image->format), D3DPOOL_DEFAULT, &texture->impl.texture, 0),
	    "Texture creation failed.");
	D3DLOCKED_RECT rect;
	kinc_microsoft_affirm(texture->impl.texture->LockRect(0, &rect, 0, 0));
	texture->impl.pitch = rect.Pitch;
	uint8_t *from = (uint8_t *)image->data;
	uint8_t *to = (uint8_t *)rect.pBits;
	// memcpy(to, from, width * height * sizeOf(format));
	for (int y = 0; y < image->height; ++y) {
		for (int x = 0; x < image->width; ++x) {
			to[rect.Pitch * y + x * 4 + 0 /* blue*/] = (from[y * image->width * 4 + x * 4 + 2]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 1 /*green*/] = (from[y * image->width * 4 + x * 4 + 1]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 2 /*  red*/] = (from[y * image->width * 4 + x * 4 + 0]); /// 255.0f;
			to[rect.Pitch * y + x * 4 + 3 /*alpha*/] = (from[y * image->width * 4 + x * 4 + 3]); /// 255.0f;
		}
	}
	kinc_microsoft_affirm(texture->impl.texture->UnlockRect(0));
}

void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {}

void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format) {
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	DWORD usage = 0;
	texture->tex_width = width;
	texture->tex_height = height;
	usage = D3DUSAGE_DYNAMIC;
	kinc_microsoft_affirm_message(device->CreateTexture(width, height, 1, usage, convert(format), D3DPOOL_DEFAULT, &texture->impl.texture, 0),
	                              "Texture creation failed.");
}

void kinc_g4_texture_destroy(kinc_g4_texture_t *texture) {
	kinc_internal_texture_unset(texture);
	texture->impl.texture->Release();
}

void kinc_internal_texture_set(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit) {
	kinc_microsoft_affirm(device->SetTexture(unit.impl.unit, texture->impl.texture));
	texture->impl.stage = unit.impl.unit;
	setTextures[texture->impl.stage] = texture;
}

void kinc_internal_texture_unset(struct kinc_g4_texture *texture) {
	if (setTextures[texture->impl.stage] == texture) {
		device->SetTexture(texture->impl.stage, nullptr);
		setTextures[texture->impl.stage] = nullptr;
	}
}

unsigned char *kinc_g4_texture_lock(kinc_g4_texture_t *texture) {
	D3DLOCKED_RECT rect;
	kinc_microsoft_affirm(texture->impl.texture->LockRect(0, &rect, 0, 0));
	texture->impl.pitch = rect.Pitch;
	return (uint8_t *)rect.pBits;
}

void kinc_g4_texture_unlock(kinc_g4_texture_t *texture) {
	kinc_microsoft_affirm(texture->impl.texture->UnlockRect(0));
}

void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

int kinc_g4_texture_stride(kinc_g4_texture_t *texture) {
	return texture->impl.pitch;
}

void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels) {}

void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level) {}
