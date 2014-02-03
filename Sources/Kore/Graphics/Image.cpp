#include "pch.h"
#include "Image.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Graphics.h>
#include "stb_image.h"
#include <stdio.h>

using namespace Kore;

int Image::sizeOf(Image::Format format) {
	switch (format) {
	case Image::RGBA32:
		return 4;
	case Image::Grey8:
		return 1;
	}
	return -1;
}

Image::Image(int width, int height, Format format, bool readable) : width(width), height(height), format(format), readable(readable) {
	data = new u8[width * height * sizeOf(format)];
}

Image::Image(const char* filename, bool readable) : format(RGBA32), readable(readable) {
	printf("Image %s\n", filename);
	FileReader file(filename);
	int size = file.size();
	int comp;
	data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
}

Image::~Image() {
	delete[] data;
}

int Image::at(int x, int y) {
	return *(int*)&((u8*)data)[width * sizeOf(format) * y + x * sizeOf(format)];
}
