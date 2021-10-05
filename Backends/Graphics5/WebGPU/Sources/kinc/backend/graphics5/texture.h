#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int unused;
} TextureUnit5Impl;

typedef struct {
    WGPUTexture texture;
} Texture5Impl;

#ifdef __cplusplus
}
#endif
