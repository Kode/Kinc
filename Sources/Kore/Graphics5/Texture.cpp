#include "Texture.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>

using namespace Kore::Graphics5;

/*Texture::Texture(kinc_g4_texture_t texture) : Image(texture.tex_width, texture.tex_height, texture.tex_depth, (Format)texture.format, false) {
    texWidth = kincTexture.tex_width;
    texHeight = kincTexture.tex_height;
    texDepth = kincTexture.tex_depth;

    free(data);
    data = nullptr;
}*/

Texture::Texture(const char *filename, bool readable) : Image(filename, readable) {
	kinc_image_t image;
	kinc_image_init(&image, data, width, height, (kinc_image_format_t)format);
	kinc_g5_texture_init_from_image(&kincTexture, &image);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1;
	kinc_image_destroy(&image);

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	kinc_g5_texture_init(&kincTexture, width, height, (kinc_image_format_t)format);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1;

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

Texture::Texture(int width, int height, int depth, Format format, bool readable) : Image(width, height, depth, format, readable) {
	kinc_g5_texture_init3d(&kincTexture, width, height, depth, (kinc_image_format_t)format);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1; // kincTexture.texDepth;

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

Texture::Texture(void *imageData, int width, int height, int format, bool readable)
    : Image(imageData, width, height, (Kore::Graphics1::Image::Format)format, readable) {
	kinc_image_t image;
	kinc_image_init(&image, imageData, width, height, (kinc_image_format_t)format);
	kinc_g5_texture_init_from_image(&kincTexture, &image);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1;
	kinc_image_destroy(&image);

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

Texture::Texture(void *imageData, int width, int height, int depth, int format, bool readable)
    : Image(imageData, width, height, depth, (Kore::Graphics1::Image::Format)format, readable) {
	kinc_image_t image;
	kinc_image_init3d(&image, imageData, width, height, depth, (kinc_image_format_t)format);
	kinc_g5_texture_init_from_image(&kincTexture, &image);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1; // kincTexture.texDepth;
	kinc_image_destroy(&image);

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

/*Texture::Texture(void *filedata, int size, const char *format, bool readable) {
    BufferReader reader(filedata, size);
    Image::init(reader, format, readable);

    kinc_image_t image;
    kinc_image_init(&image, data, width, height, (kinc_image_format_t)this->format);
    kinc_g5_texture_init_from_image(&kincTexture, &image);
    texWidth = kincTexture.texWidth;
    texHeight = kincTexture.texHeight;
    texDepth = 1;
    kinc_image_destroy(&image);

    if (!readable) {
        free(data);
        data = nullptr;
    }
}*/

Texture::Texture(Kore::Reader &reader, const char *format, bool readable) : Image(reader, format, readable) {
	kinc_image_t image;
	kinc_image_init(&image, data, width, height, (kinc_image_format_t)this->format);
	kinc_g5_texture_init_from_image(&kincTexture, &image);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1;
	kinc_image_destroy(&image);

	if (!readable) {
		free(data);
		data = nullptr;
	}
}

Texture::~Texture() {
	kinc_g5_texture_destroy(&kincTexture);
}

#ifdef KORE_ANDROID
KINC_FUNC void kinc_g4_texture_init_from_id(kinc_g4_texture_t *texture, unsigned texid);

Texture::Texture(unsigned texid) {
	kinc_g4_texture_init_from_id(&kincTexture.impl.texture, texid);
}
#endif

uint8_t *Texture::lock() {
	return kinc_g5_texture_lock(&kincTexture);
}

void Texture::unlock() {
	kinc_g5_texture_unlock(&kincTexture);
}

int Texture::stride() {
	return kinc_g5_texture_stride(&kincTexture);
}

#if defined(KORE_IOS) || defined(KORE_MACOS)
void Texture::upload(u8 *data, int stride) {
	//kinc_g5_texture_upload(&kincTexture, data, stride);
}
#endif

void Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {
	kinc_g5_texture_clear(&kincTexture, x, y, z, width, height, depth, color);
}

void Texture::generateMipmaps(int levels) {
	kinc_g5_texture_generate_mipmaps(&kincTexture, levels);
}

void Texture::setMipmap(Texture *mipmap, int level) {
	kinc_image_t image;
	kinc_image_init(&image, mipmap->data, mipmap->width, mipmap->height, (kinc_image_format_t)mipmap->format);
	kinc_g5_texture_set_mipmap(&kincTexture, &image, level);
	texWidth = kincTexture.texWidth;
	texHeight = kincTexture.texHeight;
	texDepth = 1;
	kinc_image_destroy(&image);
}
