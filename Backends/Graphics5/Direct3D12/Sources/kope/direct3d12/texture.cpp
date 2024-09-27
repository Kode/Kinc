#include "texture_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/texture.h>

uint32_t kope_d3d12_texture_resource_state_index(kope_g5_texture *texture, uint32_t mip_level, uint32_t array_layer) {
	return mip_level + (array_layer * texture->d3d12.mip_level_count);
}
