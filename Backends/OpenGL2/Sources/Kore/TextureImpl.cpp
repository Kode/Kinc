#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Log.h>
#include "ogl.h"

using namespace Kore;

namespace {
	int convert(Image::Format format) {
		switch (format) {
			case Image::RGBA32:
			default:
				return GL_RGBA;
			case Image::RGB24:
				return GL_RGB;
			case Image::Grey8:
#ifdef OPENGLES
				return GL_LUMINANCE;
#else
				return GL_RED;
#endif
		}
	}
	
	int convertInternal(Image::Format format) {
		switch (format) {
			case Image::RGBA32:
			default:
			#ifdef GL_BGRA
				return GL_BGRA;
			#else
				return GL_RGBA;
			#endif
			case Image::RGB24:
				return GL_RGB;
			case Image::Grey8:
#ifdef OPENGLES
				return GL_LUMINANCE;
#else
				return GL_RED;
#endif
		}
	}

#if 0
	int astcFormat(u8 blockX, u8 blockY) {
		switch (blockX) {
		case 4:
			switch (blockY) {
			case 4:
				return COMPRESSED_RGBA_ASTC_4x4_KHR;
			}
		case 5:
			switch (blockY) {
			case 4:
				return COMPRESSED_RGBA_ASTC_5x4_KHR;
			case 5:
				return COMPRESSED_RGBA_ASTC_5x5_KHR;
			}
		case 6:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_6x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_6x6_KHR;
			}
		case 8:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_8x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_8x6_KHR;
			case 8:
				return COMPRESSED_RGBA_ASTC_8x8_KHR;
			}
		case 10:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_10x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_10x6_KHR;
			case 8:
				return COMPRESSED_RGBA_ASTC_10x8_KHR;
			case 10:
				return COMPRESSED_RGBA_ASTC_10x10_KHR;
			}
		case 12:
			switch (blockY) {
			case 10:
				return COMPRESSED_RGBA_ASTC_12x10_KHR;
			case 12:
				return COMPRESSED_RGBA_ASTC_12x12_KHR;
			}
		}
		return 0;
	}
#endif

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
	
	if (compressed) {
#if defined(SYS_IOS)
		texWidth = Kore::max(texWidth, texHeight);
		texHeight = Kore::max(texWidth, texHeight);
		if (texWidth < 8) texWidth = 8;
		if (texHeight < 8) texHeight = 8;
#elif defined(SYS_ANDROID)
		texWidth = width;
		texHeight = height;
#endif
	}

	if (!compressed) {
		conversionBuffer = new u8[texWidth * texHeight * sizeOf(format)];
		convertImage(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
	}
	else {
		conversionBuffer = nullptr;
	}

#ifdef SYS_ANDROID
	external_oes = false;
#endif
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	if (compressed) {
#if defined(SYS_IOS)
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, texWidth, texHeight, 0, texWidth * texHeight / 2, data);
//#elif defined(SYS_ANDROID)
//		u8 blockX = internalFormat >> 8;
//		u8 blockY = internalFormat & 0xff;
//		glCompressedTexImage2D(GL_TEXTURE_2D, 0, astcFormat(blockX, blockY), texWidth, texHeight, 0, dataSize, data);
#endif
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, conversionBuffer);
		glCheckErrors();
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	
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
	else {
		if (compressed) {
			log(Kore::Warning, "Compressed images can not be readable.");
		}
		else {
			u8* data2 = new u8[texWidth * texHeight * sizeOf(format)];
			convertImage2(format, (u8*)data, width, height, data2, texWidth, texHeight);
			delete[] data;
			data = data2;
		}
	}
}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
#ifdef SYS_IOS
	texWidth = width;
	texHeight = height;
#else
	texWidth = getPower2(width);
	texHeight = getPower2(height);
#endif
	conversionBuffer = new u8[texWidth * texHeight * 4];

#ifdef SYS_ANDROID
	external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();
	
	glTexImage2D(GL_TEXTURE_2D, 0, convert(format), texWidth, texHeight, 0, convertInternal(format), GL_UNSIGNED_BYTE, data);
	glCheckErrors();

	/*if (!readable) {
		delete[] data;
		data = nullptr;
	}*/
}

#ifdef SYS_ANDROID
Texture::Texture(unsigned texid) : Image(1023, 684, Image::RGBA32, false) {
	texture = texid;
	external_oes = true;
	texWidth = 1023;
	texHeight = 684;
}
#endif

TextureImpl::~TextureImpl() {
	glDeleteTextures(1, &texture);
	delete[] conversionBuffer;
	glFlush();
}

void Texture::_set(TextureUnit unit) {
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors();
#ifdef SYS_ANDROID
	if (external_oes) {
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
		glCheckErrors();
	}
	else {
		glBindTexture(GL_TEXTURE_2D, texture);
		glCheckErrors();
	}
#else
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
#endif
}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

/*void Texture::unlock() {
	if (conversionBuffer != nullptr) {
		convertImage2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
		glBindTexture(GL_TEXTURE_2D, texture);
#ifndef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
	}
}*/

void Texture::unlock() {
	if (conversionBuffer != nullptr) {
		//convertImage2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
		glBindTexture(GL_TEXTURE_2D, texture);
		glCheckErrors();
		//glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, convertInternal(format), GL_UNSIGNED_BYTE, data);
		glCheckErrors();
	}
}

#ifdef SYS_IOS
void Texture::upload(u8* data) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, convertInternal(format), GL_UNSIGNED_BYTE, data);
	glCheckErrors();
}
#endif
