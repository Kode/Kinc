#pragma once

#include <kinc/image.h>

typedef struct {
	int index;
	bool vertex;
} TextureUnit5Impl;

typedef struct {
	void *_tex;
	void *data;
} Texture5Impl;
