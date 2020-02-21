#pragma once

#include <webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	WGPUBuffer buffer;
	int count;
} IndexBuffer5Impl;

#ifdef __cplusplus
}
#endif
