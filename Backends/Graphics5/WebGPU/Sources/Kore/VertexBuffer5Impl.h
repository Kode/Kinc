#pragma once

#include <webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	WGPUBuffer buffer;
	int count;
	int stride;
} VertexBuffer5Impl;

#ifdef __cplusplus
}
#endif
