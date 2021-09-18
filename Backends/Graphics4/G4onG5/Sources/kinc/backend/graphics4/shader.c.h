#include <kinc/graphics4/shader.h>

void kinc_g4_shader_init(kinc_g4_shader_t *shader, void *_data, size_t length, kinc_g4_shader_type_t type) {
	kinc_g5_shader_init(&shader->impl._shader, _data, length, (kinc_g5_shader_type_t)type);
}

void kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type) {
	kinc_g5_shader_init(&shader->impl._shader, NULL, 0, (kinc_g5_shader_type_t)type);
}

void kinc_g4_shader_destroy(kinc_g4_shader_t *shader) {
	kinc_g5_shader_destroy(&shader->impl._shader);
}
