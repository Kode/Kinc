#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Log.h>

using namespace Kore;

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {

}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {

}

TextureImpl::~TextureImpl() {

}

void Texture::_set(TextureUnit unit) {

}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {

}
