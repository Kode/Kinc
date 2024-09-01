#include "shader_functions.h"
#include "shader_structs.h"

void kope_d3d12_shader_init(kope_d3d12_shader *shader, const void *data, size_t size, kope_d3d12_shader_stage stage) {
	shader->size = size;
	shader->data = (uint8_t *)malloc(size);
	memcpy(shader->data, data, size);
}

void kope_d3d12_shader_destroy(kope_d3d12_shader *shader) {
	free(shader->data);
}
