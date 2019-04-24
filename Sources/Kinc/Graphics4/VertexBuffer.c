#include "pch.h"

#include "VertexBuffer.h"

void kinc_g4_vertex_element_init(kinc_g4_vertex_element_t *element, const char *name, kinc_g4_vertex_data_t data) {
	element->name = name;
	element->data = data;
}

void kinc_g4_vertex_structure_init(kinc_g4_vertex_structure_t *structure) {
	structure->size = 0;
	structure->instanced = false;
}

void kinc_g4_vertex_structure_add(kinc_g4_vertex_structure_t *structure, const char *name, kinc_g4_vertex_data_t data) {
	kinc_g4_vertex_element_init(&structure->elements[structure->size++], name, data);
}
