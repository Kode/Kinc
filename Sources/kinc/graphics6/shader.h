#pragma once

#include <kinc/global.h>
#include <kinc/backend/graphics6/shader.h>

/*! \file shader.h
    \brief Provides functions for setting up shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_shader_stage {
	KINC_G6_SHADER_STAGE_VERTEX = 0x01,
	KINC_G6_SHADER_STAGE_FRAGMENT = 0x02,
	KINC_G6_SHADER_STAGE_COMPUTE = 0x04
} kinc_g6_shader_stage_t;

typedef uint32_t kinc_g6_shader_stage_flags_t;

typedef struct kinc_g6_shader {
	kinc_g6_shader_impl_t impl;
} kinc_g6_shader_t;

typedef struct kinc_g6_shader_descriptor {
	kinc_g6_shader_stage_t stage;
	const void *code;
	uint32_t code_size;
} kinc_g6_shader_descriptor_t;

KINC_FUNC void kinc_g6_shader_init(kinc_g6_shader_t *shader, const kinc_g6_shader_descriptor_t *descriptor);
KINC_FUNC void kinc_g6_shader_destroy(kinc_g6_shader_t *shader);

#ifdef __cplusplus
}
#endif