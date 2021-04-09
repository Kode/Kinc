#pragma once

#include <kinc/image.h>

#include <objc/runtime.h>

typedef struct {
	int index;
	bool vertex;
} TextureUnit5Impl;

typedef struct {
	id _tex;
	void *data;
} Texture5Impl;
