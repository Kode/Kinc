#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include "ogl.h"

using namespace Kore;

namespace {
	int Pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int GetPower2(int i) {
		for (int power = 0; ; ++power)
			if (Pow(power) >= i) return Pow(power);
	}
}

Texture::Texture(const char* filename) : Image(filename) {
	texWidth = GetPower2(width);
	texHeight = GetPower2(height);

	u8* from = (u8*)data;
	conversionBuffer = new u8[texWidth * texHeight * 4];
	u8* to = conversionBuffer;
	for (int y = 0; y < texHeight; ++y) {
		for (int x = 0; x < texWidth; ++x) {
			to[texWidth * 4 * y + x * 4 + 0] = 0;
			to[texWidth * 4 * y + x * 4 + 1] = 0;
			to[texWidth * 4 * y + x * 4 + 2] = 0;
			to[texWidth * 4 * y + x * 4 + 3] = 0;
		}
	}
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			to[texWidth * 4 * y + x * 4 + 0] = from[y * width * 4 + x * 4 + 0];
			to[texWidth * 4 * y + x * 4 + 1] = from[y * width * 4 + x * 4 + 1];
			to[texWidth * 4 * y + x * 4 + 2] = from[y * width * 4 + x * 4 + 2];
			to[texWidth * 4 * y + x * 4 + 3] = from[y * width * 4 + x * 4 + 3];
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, to);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float color[] = { 1, 0, 0, 0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	delete[] conversionBuffer;
	conversionBuffer = nullptr;
}

Texture::Texture(int width, int height, Image::Format format) : Image(width, height, format) {

}

TextureImpl::~TextureImpl() {
	glDeleteTextures(1, &texture);
	delete[] conversionBuffer;
}

void Texture::set(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}
