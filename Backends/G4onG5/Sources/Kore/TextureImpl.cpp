#include "pch.h"

#include <Kore/Graphics/Graphics.h>

#include "TextureImpl.h"

using namespace Kore;

void Texture::init(const char* format, bool readable) {

}

Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
	
}

Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

TextureImpl::~TextureImpl() {
	unset();
}

void TextureImpl::unmipmap() {
	
}

void TextureImpl::unset() {
	
}

u8* Texture::lock() {
	return nullptr;
}

void Texture::unlock() {
	
}

void Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

int Texture::stride() {
	return 1;
}

void Texture::generateMipmaps(int levels) {}

void Texture::setMipmap(Texture* mipmap, int level) {}
