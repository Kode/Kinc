#ifndef KOPE_D3D12_TEXTURE_FUNCTIONS_HEADER
#define KOPE_D3D12_TEXTURE_FUNCTIONS_HEADER

#include <kope/graphics5/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t kope_d3d12_texture_resource_state_index(kope_g5_texture *texture, uint32_t mip_level, uint32_t array_layer);

#ifdef __cplusplus
}
#endif

#endif
