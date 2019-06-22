#include "pch.h"

#include "ogl.h"

#include <kinc/graphics4/shader.h>

#include <stdlib.h>
#include <string.h>

void kinc_g4_shader_init(kinc_g4_shader_t *shader, void *data, size_t length, kinc_g4_shader_type_t type) {
	shader->impl.length = length;
	shader->impl._glid = 0;
	shader->impl.fromSource = false;
	char* source = (char*)malloc(length + 1);
	memcpy(source, data, length);
	source[length] = 0;
	shader->impl.source = source;
}

void kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type) {
	shader->impl.source = source;
	shader->impl.length = strlen(source);
	shader->impl._glid = 0;
	shader->impl.fromSource = true;
}

void kinc_g4_shader_destroy(kinc_g4_shader_t *shader) {
	if (!shader->impl.fromSource) {
		free((void*)shader->impl.source);
	}
	shader->impl.source = NULL;
	if (shader->impl._glid != 0) {
		glDeleteShader(shader->impl._glid);
	}
}
