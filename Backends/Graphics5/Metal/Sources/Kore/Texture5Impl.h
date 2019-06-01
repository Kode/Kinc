#pragma once

#include <kinc/image.h>

#include <objc/runtime.h>

typedef struct {
	int index;
} TextureUnit5Impl;

typedef struct {
	//Texture5Impl();
	//~Texture5Impl();
	id _tex;
	id _sampler;

	//void create(int width, int height, int format, bool writable);
} Texture5Impl;
