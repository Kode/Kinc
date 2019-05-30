#pragma once

#include "pch.h"

#include <Kore/ShaderImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g4_shader_type {
	KINC_G4_SHADER_TYPE_FRAGMENT,
	KINC_G4_SHADER_TYPE_VERTEX,
	KINC_G4_SHADER_TYPE_GEOMETRY,
	KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL,
	KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION
} kinc_g4_shader_type_t;

typedef struct kinc_g4_shader {
	Kinc_G4_ShaderImpl impl;
} kinc_g4_shader_t;

void kinc_g4_shader_init(kinc_g4_shader_t *shader, void *data, size_t length, kinc_g4_shader_type_t type);
void kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type); // Beware, this is not portable
void kinc_g4_shader_destroy(kinc_g4_shader_t *shader);

#ifdef __cplusplus
}
#endif
