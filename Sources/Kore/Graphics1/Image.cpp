#include "pch.h"

#if defined(KORE_WINDOWS) && defined(KORE_VULKAN)
#include <windows.h>
#endif

#ifdef KORE_LZ4X
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxOutputSize);
#else
#include "../IO/lz4/lz4.h"
#endif
#include "Image.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>
#include <Kore/IO/Reader.h>
#include <Kore/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <string.h>

using namespace Kore;

namespace {

	bool endsWith(const char* str, const char* suffix) {
		if (!str || !suffix) return 0;
		size_t lenstr = strlen(str);
		size_t lensuffix = strlen(suffix);
		if (lensuffix > lenstr) return 0;
		return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
	}

	void loadImage(Kore::Reader& file, const char* filename, u8*& output, int& outputSize, int& width, int& height, Graphics1::ImageCompression& compression,
	               Graphics1::Image::Format& format, unsigned& internalFormat) {
		format = Graphics1::Image::RGBA32;
		if (endsWith(filename, "k")) {
			u8* data = (u8*)file.readAll();
			width = Reader::readS32LE(data + 0);
			height = Reader::readS32LE(data + 4);
			char fourcc[5];
			fourcc[0] = Reader::readS8(data + 8);
			fourcc[1] = Reader::readS8(data + 9);
			fourcc[2] = Reader::readS8(data + 10);
			fourcc[3] = Reader::readS8(data + 11);
			fourcc[4] = 0;
			if (strcmp(fourcc, "LZ4 ") == 0) {
				compression = Graphics1::ImageCompressionNone;
				internalFormat = 0;
				outputSize = width * height * 4;
				output = (u8*)malloc(outputSize);
				LZ4_decompress_safe((char*)(data + 12), (char*)output, file.size() - 12, outputSize);
			}
			else if (strcmp(fourcc, "LZ4F") == 0) {
				compression = Graphics1::ImageCompressionNone;
				internalFormat = 0;
				outputSize = width * height * 16;
				output = (u8*)malloc(outputSize);
				LZ4_decompress_safe((char*)(data + 12), (char*)output, file.size() - 12, outputSize);
				format = Graphics1::Image::RGBA128;
			}
			else if (strcmp(fourcc, "ASTC") == 0) {
				compression = Graphics1::ImageCompressionASTC;
				outputSize = width * height * 4;
				u8* astcdata = (u8*)malloc(outputSize);
				outputSize = LZ4_decompress_safe((char*)(data + 12), (char*)astcdata, file.size() - 12, outputSize);

				output = astcdata;
				u8 blockdim_x = 6;
				u8 blockdim_y = 6;
				internalFormat = (blockdim_x << 8) + blockdim_y;

				/*int index = 0;
				index += 4; // magic
				u8 blockdim_x = astcdata[index++];
				u8 blockdim_y = astcdata[index++];
				++index; // blockdim_z
				internalFormat = (blockdim_x << 8) + blockdim_y;
				u8 xsize[4];
				xsize[0] = astcdata[index++];
				xsize[1] = astcdata[index++];
				xsize[2] = astcdata[index++];
				xsize[3] = 0;
				this->width = *(unsigned*)&xsize[0];
				u8 ysize[4];
				ysize[0] = astcdata[index++];
				ysize[1] = astcdata[index++];
				ysize[2] = astcdata[index++];
				ysize[3] = 0;
				this->height = *(unsigned*)&ysize[0];
				u8 zsize[3];
				zsize[0] = astcdata[index++];
				zsize[1] = astcdata[index++];
				zsize[2] = astcdata[index++];
				u8* all = (u8*)astcdata[index];
				dataSize -= 16;
				this->data = new u8[dataSize];
				for (int i = 0; i < dataSize; ++i) {
				data[i] = all[16 + i];
				}
				free(astcdata);*/
			}
			else if (strcmp(fourcc, "DXT5") == 0) {
				compression = Graphics1::ImageCompressionDXT5;
				outputSize = width * height;
				u8* dxt5data = (u8*)malloc(outputSize);
				outputSize = LZ4_decompress_safe((char*)(data + 12), (char*)dxt5data, file.size() - 12, outputSize);

				output = dxt5data;
				internalFormat = 0;
			}
			else {
				log(Error, "Unknown fourcc in .k file.");
			}
		}
		else if (endsWith(filename, "pvr")) {
			file.readU32LE(); // version
			file.readU32LE(); // flags
			file.readU64LE(); // pixelFormat1
			file.readU32LE(); // colourSpace
			file.readU32LE(); // channelType
			u32 hh = file.readU32LE();
			u32 ww = file.readU32LE();
			file.readU32LE(); // depth
			file.readU32LE(); // numSurfaces
			file.readU32LE(); // numFaces
			file.readU32LE(); // mipMapCount
			u32 metaDataSize = file.readU32LE();

			u32 meta1fourcc = file.readU32LE();
			file.readU32LE(); // meta1key
			file.readU32LE(); // meta1size
			u32 meta1data = file.readU32LE();

			u32 meta2fourcc = file.readU32LE();
			file.readU32LE(); // meta2key
			file.readU32LE(); // meta2size
			u32 meta2data = file.readU32LE();

			int w = 0;
			int h = 0;

			if (meta1fourcc == 0) w = meta1data;
			if (meta1fourcc == 1) h = meta1data;
			if (meta2fourcc == 0) w = meta2data;
			if (meta2fourcc == 1) h = meta2data;

			width = w;
			height = h;
			compression = Graphics1::ImageCompressionPVRTC;
			internalFormat = 0;

			u8* all = (u8*)file.readAll();
			outputSize = ww * hh / 2;
			output = new u8[outputSize];
			for (int i = 0; i < outputSize; ++i) {
				output[i] = all[52 + metaDataSize + i];
			}
		}
		else if (endsWith(filename, "png")) {
			int size = file.size();
			int comp;
			compression = Graphics1::ImageCompressionNone;
			internalFormat = 0;
			output = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
			if (output == nullptr) {
				log(Error, stbi_failure_reason());
			}
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					float r = output[y * width * 4 + x * 4 + 0] / 255.0f;
					float g = output[y * width * 4 + x * 4 + 1] / 255.0f;
					float b = output[y * width * 4 + x * 4 + 2] / 255.0f;
					float a = output[y * width * 4 + x * 4 + 3] / 255.0f;
					r *= a;
					g *= a;
					b *= a;
					output[y * width * 4 + x * 4 + 0] = (u8)Kore::round(r * 255.0f);
					output[y * width * 4 + x * 4 + 1] = (u8)Kore::round(g * 255.0f);
					output[y * width * 4 + x * 4 + 2] = (u8)Kore::round(b * 255.0f);
				}
			}
			outputSize = width * height * 4;
		}
		else if (endsWith(filename, "hdr")) {
			int size = file.size();
			int comp;
			compression = Graphics1::ImageCompressionNone;
			internalFormat = 0;
			output = (u8*)stbi_loadf_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
			if (output == nullptr) {
				log(Error, stbi_failure_reason());
			}
			outputSize = width * height * 16;
			format = Graphics1::Image::RGBA128;
		}
		else {
			int size = file.size();
			int comp;
			compression = Graphics1::ImageCompressionNone;
			internalFormat = 0;
			output = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
			if (output == nullptr) {
				log(Error, stbi_failure_reason());
			}
			outputSize = width * height * 4;
		}
	}
}

int Graphics1::Image::sizeOf(Image::Format format) {
	switch (format) {
	case Image::RGBA128:
		return 16;
	case Image::RGBA32:
	case Image::BGRA32:
		return 4;
	case Image::RGBA64:
		return 8;
	case Image::A32:
		return 4;
	case Image::A16:
		return 2;
	case Image::Grey8:
		return 1;
	case Image::RGB24:
		return 3;
	}
	return -1;
}

Graphics1::Image::Image(int width, int height, Format format, bool readable) : width(width), height(height), depth(1), format(format), readable(readable) {
	compression = ImageCompressionNone;

	// If format is a floating point format
	if (format == RGBA128 || format == RGBA64 || format == A32 || format == A16) {
		hdrData = reinterpret_cast<float*>(new u8[width * height * sizeOf(format)]);
		data = nullptr;
	}
	else {
		data = new u8[width * height * sizeOf(format)];
		hdrData = nullptr;
	}
}

Graphics1::Image::Image(int width, int height, int depth, Format format, bool readable)
    : width(width), height(height), depth(depth), format(format), readable(readable) {
	compression = ImageCompressionNone;

	// If format is a floating point format
	if (format == RGBA128 || format == RGBA64 || format == A32 || format == A16) {
		hdrData = reinterpret_cast<float*>(new u8[width * height * depth * sizeOf(format)]);
		data = nullptr;
	}
	else {
		data = new u8[width * height * depth * sizeOf(format)];
		hdrData = nullptr;
	}
}

Graphics1::Image::Image(const char* filename, bool readable) : depth(1), format(RGBA32), readable(readable) {
	FileReader reader(filename);
	init(reader, filename, readable);
}

Graphics1::Image::Image(Reader& reader, const char* format, bool readable) : depth(1), format(RGBA32), readable(readable) {
	init(reader, format, readable);
}

Graphics1::Image::Image(void* data, int width, int height, Format format, bool readable)
    : width(width), height(height), depth(1), format(format), readable(readable) {
	compression = ImageCompressionNone;
	bool isFloat = format == RGBA128 || format == RGBA64 || format == A32 || format == A16;
	if (isFloat) {
		this->hdrData = (float*)data;
	}
	else {
		this->data = (u8*)data;
	}
}

Graphics1::Image::Image(void* data, int width, int height, int depth, Format format, bool readable)
    : width(width), height(height), depth(depth), format(format), readable(readable) {
	compression = ImageCompressionNone;
	bool isFloat = format == RGBA128 || format == RGBA64 || format == A32 || format == A16;
	if (isFloat) {
		this->hdrData = (float*)data;
	}
	else {
		this->data = (u8*)data;
	}
}

Graphics1::Image::Image() : depth(1), format(RGBA32), readable(false) {}

void Graphics1::Image::init(Kore::Reader& file, const char* filename, bool readable) {
	u8* imageData;
	loadImage(file, filename, imageData, dataSize, width, height, compression, this->format, internalFormat);
	bool isFloat = format == RGBA128 || format == RGBA64 || format == A32 || format == A16;
	if (isFloat) {
		hdrData = (float*)imageData;
	}
	else {
		data = imageData;
	}
}

Graphics1::Image::~Image() {
	if (readable) {
		if (format == RGBA128 || format == RGBA64 || format == A32 || format == A16) {
			delete[] hdrData;
			hdrData = nullptr;
		}
		else {
			delete[] data;
			data = nullptr;
		}
	}
}

int Graphics1::Image::at(int x, int y) {
	if (data == nullptr)
		return 0;
	else
		return *(int*)&((u8*)data)[width * sizeOf(format) * y + x * sizeOf(format)];
}
