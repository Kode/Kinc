#ifndef KOPE_G5_TEXTURE_HEADER
#define KOPE_G5_TEXTURE_HEADER

#include <kope/global.h>

#include "api.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/texture_structs.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/texture_structs.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_g5_texture {
	KOPE_G5_IMPL(texture);
} kope_g5_texture;

#ifdef __cplusplus
}
#endif

#endif
