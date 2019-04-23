#include "pch.h"

#include "ogl.h"

#include <Kinc/Graphics4/Shader.h>

#include <stdlib.h>
#include <string.h>

void Kinc_G4_Shader_Create(Kinc_G4_Shader *shader, void *data, size_t length, Kinc_G4_ShaderType type) {
	shader->impl.length = length;
	shader->impl._glid = 0;
	shader->impl.fromSource = false;
	char* source = (char*)malloc(length + 1);
	memcpy(source, data, length);
	source[length] = 0;
	shader->impl.source = source;
}

void Kinc_G4_Shader_CreateFromSource(Kinc_G4_Shader* shader, const char* source, Kinc_G4_ShaderType type) {
	shader->impl.source = source;
	shader->impl.length = strlen(source);
	shader->impl._glid = 0;
	shader->impl.fromSource = true;
}

void Kinc_G4_Shader_Destroy(Kinc_G4_Shader *shader) {
	if (!shader->impl.fromSource) {
		free((void*)shader->impl.source);
	}
	shader->impl.source = NULL;
	if (shader->impl._glid != 0) {
		glDeleteShader(shader->impl._glid);
	}
}
