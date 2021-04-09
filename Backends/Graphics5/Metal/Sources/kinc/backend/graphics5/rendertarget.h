#pragma once

#include <objc/runtime.h>

typedef struct {
	id _tex;
	id _texReadback;
	id _depthTex;
} RenderTarget5Impl;
