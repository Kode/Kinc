#include "pch.h"

#include "Texture5Impl.h"

#include <Kore/Graphics5/Graphics.h>

using namespace Kore;

static const int textureCount = 16;

void Graphics5::Texture::_init(const char* format, bool readable) {

}

Graphics5::Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {

}

Graphics5::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

Texture5Impl::~Texture5Impl() {
	unset();
}

void Texture5Impl::unmipmap() {
	mipmap = false;
}

void Graphics5::Texture::_set(TextureUnit unit) {

}

void Texture5Impl::unset() {

}

u8* Graphics5::Texture::lock() {
	return nullptr;
}

void Graphics5::Texture::unlock() {

}

void Graphics5::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

int Graphics5::Texture::stride() {
	return 32;
}

void Graphics5::Texture::generateMipmaps(int levels) {}

void Graphics5::Texture::setMipmap(Texture* mipmap, int level) {}
