#include "pch.h"

#include <Kinc/Graphics5/Shader.h>
#include <Kinc/Math/Core.h>

#include <Kinc/Graphics4/Shader.h>

void kinc_g5_shader_init(kinc_g5_shader_t *shader, void *source, int length, kinc_g5_shader_type_t type) {
	kinc_g4_shader_init(&shader->impl.shader, source, length, (kinc_g4_shader_type_t)type);
}
