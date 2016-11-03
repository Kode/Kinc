#include "pch.h"

#include "../IO/snappy/snappy.h"
#include "Image.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/IO/FileReader.h>
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

int Image::sizeOf(Image::Format format) {
	switch (format) {
	case Image::RGBA128:
		return 16;
	case Image::RGBA32:
		return 4;
	case Image::Grey8:
		return 1;
	}
	return -1;
}

Image::Image(int width, int height, Format format, bool readable) : width(width), height(height), depth(1), format(format), readable(readable) {
	compressed = false;
	data = new u8[width * height * sizeOf(format)];
}

Image::Image(int width, int height, int depth, Format format, bool readable) : width(width), height(height), depth(depth), format(format), readable(readable) {
	compressed = false;
	data = new u8[width * height * depth * sizeOf(format)];
}

Image::Image(const char* filename, bool readable) {
	const char* fileExt = strrchr(filename, '.');
	if (fileExt != nullptr) 
		++fileExt;
	else 
		fileExt = "";
	FileReader reader(filename);
	initFromReader(&reader, fileExt, readable);
}

void Image::initFromReader(Reader* reader, const char* fileExt, bool readable) {
	this->depth = 1;
	this->format = RGBA32;
	this->readable = readable;

	if (0 == strcmp(fileExt, "k")) {
		u8* data = (u8*)reader->readAll();
		width = Reader::readS32LE(data + 0);
		height = Reader::readS32LE(data + 4);
		char fourcc[5];
		fourcc[0] = Reader::readS8(data + 8);
		fourcc[1] = Reader::readS8(data + 9);
		fourcc[2] = Reader::readS8(data + 10);
		fourcc[3] = Reader::readS8(data + 11);
		fourcc[4] = 0;
		if (strcmp(fourcc, "SNAP") == 0) {
			size_t length;
			snappy::GetUncompressedLength((char*)(data + 12), reader->size() - 12, &length);
			this->data = (u8*)malloc(length);
			snappy::RawUncompress((char*)(data + 12), reader->size() - 12, (char*)this->data);
		}
	}
	else if (0 == strcmp(fileExt, "pvr")) {
		u32 version = reader->readU32LE();
		u32 flags = reader->readU32LE();
		u64 pixelFormat1 = reader->readU64LE();
		u32 colourSpace = reader->readU32LE();
		u32 channelType = reader->readU32LE();
		u32 height = reader->readU32LE();
		u32 width = reader->readU32LE();
		u32 depth = reader->readU32LE();
		u32 numSurfaces = reader->readU32LE();
		u32 numFaces = reader->readU32LE();
		u32 mipMapCount = reader->readU32LE();
		u32 metaDataSize = reader->readU32LE();
		
		u32 meta1fourcc = reader->readU32LE();
		u32 meta1key = reader->readU32LE();
		u32 meta1size = reader->readU32LE();
		u32 meta1data = reader->readU32LE();
		
		u32 meta2fourcc = reader->readU32LE();
		u32 meta2key = reader->readU32LE();
		u32 meta2size = reader->readU32LE();
		u32 meta2data = reader->readU32LE();
		
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
		
		u8* all = (u8*)reader->readAll();
		dataSize = width * height / 2;
		data = new u8[dataSize];
		for (int i = 0; i < dataSize; ++i) {
			data[i] = all[52 + metaDataSize + i];
		}
	}
	else if (0 == strcmp(fileExt, "astc")) {
		u32 magic = reader->readU32LE();
    	u8 blockdim_x = reader->readU8();
    	u8 blockdim_y = reader->readU8();
    	u8 blockdim_z = reader->readU8();
    	internalFormat = (blockdim_x << 8) + blockdim_y;
    	u8 xsize[4];
    	xsize[0] = reader->readU8();
    	xsize[1] = reader->readU8();
    	xsize[2] = reader->readU8();
    	xsize[3] = 0;
    	this->width = *(unsigned*)&xsize[0];
    	u8 ysize[4];
    	ysize[0] = reader->readU8();
    	ysize[1] = reader->readU8();
    	ysize[2] = reader->readU8();
    	ysize[3] = 0;
    	this->height = *(unsigned*)&ysize[0];
    	u8 zsize[3];
    	zsize[0] = reader->readU8();
    	zsize[1] = reader->readU8();
    	zsize[2] = reader->readU8();
    	u8* all = (u8*)reader->readAll();
		dataSize = reader->size() - 16;
		data = new u8[dataSize];
		for (int i = 0; i < dataSize; ++i) {
			data[i] = all[16 + i];
		}
	}
	else if (0 == strcmp(fileExt, "png")) {
		int size = reader->size();
		int comp;
		compressed = false;
		internalFormat = 0;
		data = stbi_load_from_memory((u8*)reader->readAll(), size, &width, &height, &comp, 4);
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
	else if (0 == strcmp(fileExt, "hdr")) {
		int size = reader->size();
		int comp;
		compressed = false;
		internalFormat = 0;
		hdrData = stbi_loadf_from_memory((u8*)reader->readAll(), size, &width, &height, &comp, 4);
		dataSize = width * height * 16;
		format = RGBA128;
	}
	else {
		int size = reader->size();
		int comp;
		compressed = false;
		internalFormat = 0;
		data = stbi_load_from_memory((u8*)reader->readAll(), size, &width, &height, &comp, 4);
		dataSize = width * height * 4;
	}
}

Image::~Image() {
	if (readable) {
		if (format == RGBA128) {
			delete[] hdrData;
			hdrData = nullptr;
		}
		else {
			delete[] data;
			data = nullptr;
		}
	}
}

int Image::at(int x, int y) {
	if (data == nullptr)
		return 0;
	else
		return *(int*)&((u8*)data)[width * sizeOf(format) * y + x * sizeOf(format)];
}
