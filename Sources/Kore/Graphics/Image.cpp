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

Image::Image(int width, int height, Format format) : width(width), height(height), format(format) {
	data = new u8[width * height * sizeOf(format)];
}

Image::Image(const char* filename) : format(RGBA32) {
	printf("Image %s\n", filename);
	FileReader file(filename);
	int size = file.size();
	int comp;
	data = stbi_load_from_memory((u8*)file.readAll(), size, &width, &height, &comp, 4);
}

Image::~Image() {
	delete data;
}
