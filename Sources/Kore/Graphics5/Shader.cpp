#include "Shader.h"

using namespace Kore::Graphics5;

Shader::Shader(void *source, int length, ShaderType type) {
	kinc_g5_shader_init(&kincShader, source, length, (kinc_g5_shader_type_t)type);
}

Shader::~Shader() {
	kinc_g5_shader_destroy(&kincShader);
}
