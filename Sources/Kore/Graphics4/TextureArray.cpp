#include <Kore/Graphics4/TextureArray.h>

Kore::Graphics4::TextureArray::TextureArray(Image **textures, int count) {
	kinc_image_t images[16];
	for (int i = 0; i < count; ++i) {
		kinc_image_init(&images[i], textures[i]->data, textures[i]->width, textures[i]->height, (kinc_image_format_t)textures[i]->format);
	}
	kinc_g4_texture_array_init(&kincArray, images, count);
	for (int i = 0; i < count; ++i) {
		kinc_image_destroy(&images[i]);
	}
}

Kore::Graphics4::TextureArray::~TextureArray() {
	kinc_g4_texture_array_destroy(&kincArray);
}
