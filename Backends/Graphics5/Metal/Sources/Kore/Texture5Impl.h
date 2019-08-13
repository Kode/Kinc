#pragma once

#include <kinc/image.h>

#include <objc/runtime.h>

typedef struct {
	int index;
} TextureUnit5Impl;

typedef struct {
	id _tex;
	id _sampler;
	void *data;
} Texture5Impl;
