#pragma once

#include <objc/runtime.h>

typedef struct {
	id _tex;
	id _sampler;
    id _samplerDesc;
	id _depthTex;
} RenderTarget5Impl;
