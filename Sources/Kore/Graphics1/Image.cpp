#include "pch.h"

#if defined(KORE_WINDOWS) && defined(KORE_VULKAN)
#include <windows.h>
#endif

#ifdef KORE_LZ4X
int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxOutputSize);
#else
#include <kinc/io/lz4/lz4.h>
#endif
#include "Image.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/IO/BufferReader.h>
#include <Kore/IO/FileReader.h>
#include <Kore/IO/Reader.h>
#include <Kore/Log.h>

#include <kinc/image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

namespace {
	static int read_callback(void *user_data, void *data, size_t size) {
		Kore::Reader *file = (Kore::Reader *)user_data;
		return file->read(data, (int)size);
	}

	static size_t size_callback(void *user_data) {
		Kore::Reader *file = (Kore::Reader *)user_data;
		return (size_t)file->size();
	}

	static int pos_callback(void *user_data) {
		Kore::Reader *file = (Kore::Reader *)user_data;
		return file->pos();
	}

	static void seek_callback(void *user_data, int pos) {
		Kore::Reader *file = (Kore::Reader *)user_data;
		file->seek(pos);
	}

	void loadImage(Kore::Reader& file, const char* filename, u8*& output, int& outputSize, int& width, int& height, Graphics1::ImageCompression& compression,
	               Graphics1::Image::Format& format, unsigned& internalFormat) {
		kinc_image_read_callbacks_t callbacks;
		callbacks.read = read_callback;
		callbacks.size = size_callback;
		callbacks.pos = pos_callback;
		callbacks.seek = seek_callback;

		size_t size = kinc_image_size_from_callbacks(callbacks, &file, filename);
		file.seek(0);
		output = new u8[size];
		kinc_image_t image;
		kinc_image_init_from_callbacks(&image, output, callbacks, &file, filename);
		outputSize = (int)size;
		width = image.width;
		height = image.height;
		compression = (Graphics1::ImageCompression)image.compression;
		format = (Graphics1::Image::Format)image.format;
		internalFormat = image.internal_format;
		kinc_image_destroy(&image);
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
	data = new u8[width * height * sizeOf(format)];
}

Graphics1::Image::Image(int width, int height, int depth, Format format, bool readable)
    : width(width), height(height), depth(depth), format(format), readable(readable) {
	compression = ImageCompressionNone;
	data = new u8[width * height * depth * sizeOf(format)];
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
	this->data = data;
}

Graphics1::Image::Image(void* data, int width, int height, int depth, Format format, bool readable)
    : width(width), height(height), depth(depth), format(format), readable(readable) {
	compression = ImageCompressionNone;
	this->data = data;
}

Graphics1::Image::Image() : depth(1), format(RGBA32), readable(false) {}

void Graphics1::Image::init(Kore::Reader& file, const char* filename, bool readable) {
	u8* imageData;
	loadImage(file, filename, imageData, dataSize, width, height, compression, this->format, internalFormat);
	data = imageData;
}

Graphics1::Image::~Image() {
	if (readable) {
		free(data);
		data = nullptr;
	}
}

int Graphics1::Image::at(int x, int y) {
	if (data == nullptr)
		return 0;
	else
		return *(int*)&((u8*)data)[width * sizeOf(format) * y + x * sizeOf(format)];
}

u8* Graphics1::Image::getPixels() {
	return (u8*)data;
}
