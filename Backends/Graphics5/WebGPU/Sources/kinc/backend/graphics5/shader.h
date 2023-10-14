#pragma once

#include <webgpu/webgpu.h>

#ifdef __cplusplus
extern "C" {
#endif

struct WGPUShaderModuleImpl;

typedef struct {
#ifdef KINC_KONG
	char entry_name[256];
#else
	WGPUShaderModule module;
#endif
} Shader5Impl;

#ifdef __cplusplus
}
#endif
