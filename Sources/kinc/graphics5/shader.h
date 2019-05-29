#pragma once

#include "Texture.h"
#include "VertexStructure.h"

#include <Kinc/Math/Matrix.h>

#include <Kore/PipelineState5Impl.h>
#include <Kore/Shader5Impl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g5_shader_type {
	KINC_G5_SHADER_TYPE_FRAGMENT,
	KINC_G5_SHADER_TYPE_VERTEX,
	KINC_G5_SHADER_TYPE_GEOMETRY,
	KINC_G5_SHADER_TYPE_TESSELLATION_CONTROL,
	KINC_G5_SHADER_TYPE_TESSELLATION_EVALUATION
} kinc_g5_shader_type_t;

typedef struct kinc_g5_shader {
	Shader5Impl impl;
} kinc_g5_shader_t;

void kinc_g5_shader_init(kinc_g5_shader_t *shader, void *source, size_t length, kinc_g5_shader_type_t type);
void kinc_g5_shader_destroy(kinc_g5_shader_t *shader);

#ifdef __cplusplus
}
#endif
