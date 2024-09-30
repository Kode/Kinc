#include "texture_functions.h"

#include "d3d12unit.h"

#include <kope/graphics5/texture.h>

uint32_t kope_d3d12_texture_resource_state_index(kope_g5_texture *texture, uint32_t mip_level, uint32_t array_layer) {
	return mip_level + (array_layer * texture->d3d12.mip_level_count);
}

void kope_d3d12_texture_set_name(kope_g5_texture *texture, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	texture->d3d12.resource->SetName(wstr);
}
