#include "pch.h"

#if defined(KORE_WINDOWS) && defined(KORE_VULKAN)
#include <windows.h>
#endif

#include <Kinc/IO/lz4/lz4.h>
#include "Image.h"

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/IO/FileReader.h>
#include <Kinc/Log.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <string.h>

#include <stdint.h>

uint8_t buffer[4096 * 4096 * 4];

static _Bool endsWith(const char* str, const char* suffix) {
	if (str == NULL || suffix == NULL) return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr) return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static void loadImage(Kinc_FileReader *file, const char *filename, uint8_t *output, int *outputSize, int *width, int *height, Kinc_ImageCompression *compression,
	            Kinc_ImageFormat *format, unsigned *internalFormat) {
	*format = KINC_IMAGE_FORMAT_RGBA32;
	if (endsWith(filename, "k")) {
		uint8_t data[4];
		Kinc_FileReader_Read(file, data, 4);
		*width = Kinc_ReadS32LE(data);
		Kinc_FileReader_Read(file, data, 4);
		*height = Kinc_ReadS32LE(data);

		char fourcc[5];
		Kinc_FileReader_Read(file, fourcc, 4);
		fourcc[4] = 0;

		int compressedSize = Kinc_FileReader_Size(file) - 12;

		if (strcmp(fourcc, "LZ4 ") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_NONE;
			internalFormat = 0;
			*outputSize = *width * *height * 4;
			if (output == NULL) {
				return;
			}
			Kinc_FileReader_Read(file, buffer, compressedSize);
			LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, outputSize);
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_NONE;
			internalFormat = 0;
			*outputSize = *width * *height * 16;
			if (output == NULL) {
				return;
			}
			Kinc_FileReader_Read(file, buffer, compressedSize);
			LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, outputSize);
			format = KINC_IMAGE_FORMAT_RGBA128;
		}
		else if (strcmp(fourcc, "ASTC") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_ASTC;
			*outputSize = *width * *height * 4;
			if (output == NULL) {
				return;
			}
			Kinc_FileReader_Read(file, buffer, compressedSize);
			*outputSize = LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, outputSize);

			uint8_t blockdim_x = 6;
			uint8_t blockdim_y = 6;
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
			*compression = KINC_IMAGE_COMPRESSION_DXT5;
			*outputSize = *width * *height;
			if (output == NULL) {
				return;
			}
			Kinc_FileReader_Read(file, buffer, compressedSize);
			*outputSize = LZ4_decompress_safe((char*)(data + 12), (char*)output, compressedSize, outputSize);
			internalFormat = 0;
		}
		else {
			Kinc_Log(KINC_LOG_LEVEL_ERROR, "Unknown fourcc in .k file.");
		}
	}
	else if (endsWith(filename, "pvr")) {
		uint8_t data[4];
		Kinc_FileReader_Read(file, data, 4); // version
		Kinc_FileReader_Read(file, data, 4); // flags
		Kinc_FileReader_Read(file, data, 4); // pixelFormat1
		Kinc_FileReader_Read(file, data, 4); // colourSpace
		Kinc_FileReader_Read(file, data, 4); // channelType
		Kinc_FileReader_Read(file, data, 4);
		uint32_t hh = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4);
		uint32_t ww = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // depth
		Kinc_FileReader_Read(file, data, 4); // numSurfaces
		Kinc_FileReader_Read(file, data, 4); // numFaces
		Kinc_FileReader_Read(file, data, 4); // mipMapCount
		Kinc_FileReader_Read(file, data, 4);
		uint32_t metaDataSize = Kinc_ReadU32LE(data);

		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta1fourcc = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // meta1key
		Kinc_FileReader_Read(file, data, 4); // meta1size
		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta1data = Kinc_ReadU32LE(data);

		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta2fourcc = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // meta2key
		Kinc_FileReader_Read(file, data, 4); // meta2size
		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta2data = Kinc_ReadU32LE(data);

		int w = 0;
		int h = 0;

		if (meta1fourcc == 0) w = meta1data;
		if (meta1fourcc == 1) h = meta1data;
		if (meta2fourcc == 0) w = meta2data;
		if (meta2fourcc == 1) h = meta2data;

		*width = w;
		*height = h;
		compression = KINC_IMAGE_COMPRESSION_PVRTC;
		internalFormat = 0;

		uint8_t *all = (u8*)file.readAll();
		*outputSize = ww * hh / 2;
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

int Kinc_ImageFormat_SizeOf(Kinc_ImageFormat format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
		return 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_RGB24:
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

u8* Graphics1::Image::getPixels() {
	return data != nullptr ? data : reinterpret_cast<u8*>(hdrData);
}
