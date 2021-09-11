#include "vertexlayout.h"

void kinc_g6_vertex_layout_init(kinc_g6_vertex_layout_t *layout) {
	layout->attribute_count = 0;
	layout->instanced = false;
	layout->stride = 0;
}

void kinc_g6_vertex_layout_add(kinc_g6_vertex_layout_t *layout, uint64_t offset, uint32_t location, kinc_g6_vertex_format_t format) {
	layout->attributes[layout->attribute_count].format = format;
	layout->attributes[layout->attribute_count].location = location;
	layout->attributes[layout->attribute_count].offset = offset;
	++(layout->attribute_count);
}