#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Log.h>

using namespace Kore;

namespace {
	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0; ; ++power)
			if (pow(power) >= i) return pow(power);
	}
}

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
	texWidth = getPower2(width);
	texHeight = getPower2(height);

}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;

}

TextureImpl::~TextureImpl() {

}

void Texture::set(TextureUnit unit) {

}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	
}

#ifdef SYS_IOS
void Texture::upload(u8* data) {

}
#endif
