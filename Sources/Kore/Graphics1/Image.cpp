#include "pch.h"

#if defined(KORE_WINDOWS) && defined(KORE_VULKAN)
#include <windows.h>
#endif

#include "../IO/snappy/snappy.h"
#include "../IO/lz4/lz4.h"
#include "Image.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/IO/FileReader.h>
#include <Kore/IO/BufferReader.h>
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
	case Image::Grey8:
		return 1;
	case Image::RGB24:
		return 3;
	}
	return -1;
}

Graphics1::Image::Image(int width, int height, Format format, bool readable) : width(width), height(height), depth(1), format(format), readable(readable) {
	compressed = false;
	data = new u8[width * height * sizeOf(format)];
}

Graphics1::Image::Image(int width, int height, int depth, Format format, bool readable) : width(width), height(height), depth(depth), format(format), readable(readable) {
	compressed = false;
	data = new u8[width * height * depth * sizeOf(format)];
}

Graphics1::Image::Image(const char* filename, bool readable) : depth(1), format(RGBA32), readable(readable) {
	FileReader reader(filename);
	init(reader, filename, readable);
}

Graphics1::Image::Image(Reader& reader, const char* format, bool readable) : depth(1), format(RGBA32), readable(readable) {
	init(reader, format, readable);
}

Graphics1::Image::Image(void* data, int width, int height, Format format, bool readable) : width(width), height(height), depth(1), format(format), readable(readable) {
	compressed = false;
	bool isFloat = format == RGBA128 || format == RGBA64 || format == A32;
    if (isFloat) {
        this->hdrData = (float*)data;
    }
    else {
        this->data = (u8*)data;
    }
}

Graphics1::Image::Image() : depth(1), format(RGBA32), readable(false) {}

void Graphics1::Image::init(Kore::Reader& file, const char* format, bool readable) {
	if (endsWith(format, "k")) {
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
			compressed = false;
			internalFormat = 0;
			dataSize = width * height * 4;
			this->data = (u8*)malloc(dataSize);
			LZ4_decompress_safe((char*)(data + 12), (char*)this->data, file.size() - 12, dataSize);
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			compressed = false;
			internalFormat = 0;
			dataSize = width * height * 16;
			this->hdrData = (float*)malloc(dataSize);
			LZ4_decompress_safe((char*)(data + 12), (char*)this->hdrData, file.size() - 12, dataSize);
			this->format = RGBA128;
		}
		//else if (strcmp(fourcc, "SNAP") == 0) {
		//	compressed = false;
		//	internalFormat = 0;
		//	size_t length;
		//	snappy::GetUncompressedLength((char*)(data + 12), file.size() - 12, &length);
		//	dataSize = static_cast<int>(length);
		//	this->data = (u8*)malloc(length);
		//	snappy::RawUncompress((char*)(data + 12), file.size() - 12, (char*)this->data);
		//}
		else {
			log(Error, "Unknown fourcc in .k file.");
		}
	}
	else if (endsWith(format, "pvr")) {
		file.readU32LE(); // version
		file.readU32LE(); // flags
		file.readU64LE(); // pixelFormat1
		file.readU32LE(); // colourSpace
		file.readU32LE(); // channelType
		u32 height = file.readU32LE();
		u32 width = file.readU32LE();
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

		this->width = w;
		this->height = h;
		compressed = true;
		internalFormat = 0;

		u8* all = (u8*)file.readAll();
		dataSize = width * height / 2;
		data = new u8[dataSize];
		for (int i = 0; i < dataSize; ++i) {
			data[i] = all[52 + metaDataSize + i];
		}
	}
	else if (endsWith(format, "astc")) {
		file.readU32LE(); // magic
		u8 blockdim_x = file.readU8();
		u8 blockdim_y = file.readU8();
		file.readU8(); // blockdim_z
		internalFormat = (blockdim_x << 8) + blockdim_y;
		u8 xsize[4];
		xsize[0] = file.readU8();
		xsize[1] = file.readU8();
		xsize[2] = file.readU8();
		xsize[3] = 0;
		this->width = *(unsigned*)&xsize[0];
		u8 ysize[4];
		ysize[0] = file.readU8();
		ysize[1] = file.readU8();
		ysize[2] = file.readU8();
		ysize[3] = 0;
		this->height = *(unsigned*)&ysize[0];
		u8 zsize[3];
		zsize[0] = file.readU8();
		zsize[1] = file.readU8();
		zsize[2] = file.readU8();
		u8* all = (u8*)file.readAll();
		dataSize = file.size() - 16;
		data = new u8[dataSize];
		for (int i = 0; i < dataSize; ++i) {
			data[i] = all[16 + i];
		}
	}
	else if (endsWith(format, "png")) {
		int size = file.size();
		int comp;
		compressed = false;
		internalFormat = 0;
		data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
		if (data == nullptr) {
			log(Error, stbi_failure_reason());
		}
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				float r = data[y * width * 4 + x * 4 + 0] / 255.0f;
				float g = data[y * width * 4 + x * 4 + 1] / 255.0f;
				float b = data[y * width * 4 + x * 4 + 2] / 255.0f;
				float a = data[y * width * 4 + x * 4 + 3] / 255.0f;
				r *= a;
				g *= a;
				b *= a;
				data[y * width * 4 + x * 4 + 0] = (u8)round(r * 255.0f);
				data[y * width * 4 + x * 4 + 1] = (u8)round(g * 255.0f);
				data[y * width * 4 + x * 4 + 2] = (u8)round(b * 255.0f);
			}
		}
		dataSize = width * height * 4;
	}
	else if (endsWith(format, "hdr")) {
		int size = file.size();
		int comp;
		compressed = false;
		internalFormat = 0;
		hdrData = stbi_loadf_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
		if (hdrData == nullptr) {
			log(Error, stbi_failure_reason());
		}
		dataSize = width * height * 16;
		this->format = RGBA128;
	}
	else {
		int size = file.size();
		int comp;
		compressed = false;
		internalFormat = 0;
		data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
		if (data == nullptr) {
			log(Error, stbi_failure_reason());
		}
		dataSize = width * height * 4;
	}
}

Graphics1::Image::~Image() {
	if (readable) {
		if (format == RGBA128 || format == RGBA64 || format == A32) {
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
