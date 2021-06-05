#include "Shader.h"

using namespace Kore;
using namespace Kore::Graphics4;

Shader::Shader(void *data, int length, ShaderType type) {
	kinc_g4_shader_init(&kincShader, data, length, (kinc_g4_shader_type_t)type);
}

Shader::Shader(const char *source, ShaderType type) {
	kinc_g4_shader_init_from_source(&kincShader, source, (kinc_g4_shader_type_t)type);
}

Shader::~Shader() {
	kinc_g4_shader_destroy(&kincShader);
}
