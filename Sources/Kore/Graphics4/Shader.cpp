#include "pch.h"

#include "Shader.h"

using namespace Kore;
using namespace Kore::Graphics4;

Shader::Shader(void* data, int length, ShaderType type) {
	Kinc_G4_Shader_Create(&kincShader, data, length, (Kinc_G4_ShaderType)type);
}

Shader::Shader(const char* source, ShaderType type) {
	Kinc_G4_Shader_CreateFromSource(&kincShader, source, (Kinc_G4_ShaderType)type);
}

Shader::~Shader() {
	Kinc_G4_Shader_Destroy(&kincShader);
}
