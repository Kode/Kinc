#include "texture.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/texture_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/texture_functions.h>
#endif

void kope_g5_texture_set_name(kope_g5_texture *texture, const char *name) {
	KOPE_G5_CALL2(texture_set_name, texture, name);
}
