#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include "ogl.h"

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

	void convertImage(Image::Format format, u8* from, int fw, int fh, u8* to, int tw, int th) {
		switch (format) {
		case Image::RGBA32:
			for (int y = 0; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					to[tw * 4 * y + x * 4 + 0] = 0;
					to[tw * 4 * y + x * 4 + 1] = 0;
					to[tw * 4 * y + x * 4 + 2] = 0;
					to[tw * 4 * y + x * 4 + 3] = 0;
				}
			}
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					to[tw * 4 * y + x * 4 + 0] = from[y * fw * 4 + x * 4 + 0];
					to[tw * 4 * y + x * 4 + 1] = from[y * fw * 4 + x * 4 + 1];
					to[tw * 4 * y + x * 4 + 2] = from[y * fw * 4 + x * 4 + 2];
					to[tw * 4 * y + x * 4 + 3] = from[y * fw * 4 + x * 4 + 3];
				}
			}
			break;
		case Image::Grey8:
			for (int y = 0; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					to[tw * y + x] = 0;
				}
			}
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					to[tw * y + x] = from[y * fw + x];
				}
			}
			break;
		}
	}
	
	void convertImage2(Image::Format format, u8* from, int fw, int fh, u8* to, int tw, int th) {
		switch (format) {
			case Image::RGBA32:
				for (int y = 0; y < th; ++y) {
					for (int x = 0; x < tw; ++x) {
						to[tw * 4 * y + x * 4 + 0] = 0;
						to[tw * 4 * y + x * 4 + 1] = 0;
						to[tw * 4 * y + x * 4 + 2] = 0;
						to[tw * 4 * y + x * 4 + 3] = 0;
					}
				}
				for (int y = 0; y < fh; ++y) {
					for (int x = 0; x < fw; ++x) {
						to[tw * 4 * y + x * 4 + 2] = from[y * fw * 4 + x * 4 + 0]; // blue
						to[tw * 4 * y + x * 4 + 1] = from[y * fw * 4 + x * 4 + 1]; // green
						to[tw * 4 * y + x * 4 + 0] = from[y * fw * 4 + x * 4 + 2]; // red
						to[tw * 4 * y + x * 4 + 3] = from[y * fw * 4 + x * 4 + 3]; // alpha
					}
				}
				break;
			case Image::Grey8:
				for (int y = 0; y < th; ++y) {
					for (int x = 0; x < tw; ++x) {
						to[tw * y + x] = 0;
					}
				}
				for (int y = 0; y < fh; ++y) {
					for (int x = 0; x < fw; ++x) {
						to[tw * y + x] = from[y * fw + x];
					}
				}
				break;
		}
	}
}

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
	texWidth = getPower2(width);
	texHeight = getPower2(height);

	conversionBuffer = new u8[texWidth * texHeight * sizeOf(format)];
	convertImage(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float color[] = { 1, 0, 0, 0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	
	delete[] conversionBuffer;
	conversionBuffer = nullptr;

	if (!readable) {
		delete[] data;
		data = nullptr;
	}
}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = getPower2(width);
	texHeight = getPower2(height);
	conversionBuffer = new u8[texWidth * texHeight * 4];

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/*if (!readable) {
		delete[] data;
		data = nullptr;
	}*/
}

TextureImpl::~TextureImpl() {
	glDeleteTextures(1, &texture);
	delete[] conversionBuffer;
}

void Texture::set(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	if (conversionBuffer != nullptr) {
		convertImage2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
		glBindTexture(GL_TEXTURE_2D, texture);
#ifndef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
	}
}
