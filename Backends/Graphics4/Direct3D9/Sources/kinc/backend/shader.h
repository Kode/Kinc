#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ShaderRegister {
	char name[128];
	uint8_t regtype;
	uint8_t regindex;
	uint8_t regcount;
};

struct ShaderAttribute {
	char name[128];
	int index;
};

typedef struct {
	struct ShaderRegister reg;
	int shaderType; // 0: Vertex, 1: Fragment
} kinc_g4_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_g4_texture_unit_impl_t;

#define KINC_INTERNAL_MAX_CONSTANTS 64
#define KINC_INTERNAL_MAX_ATTRIBUTES 64

typedef struct {
	struct ShaderRegister constants[KINC_INTERNAL_MAX_CONSTANTS];
	struct ShaderAttribute attributes[KINC_INTERNAL_MAX_ATTRIBUTES];
	void *shader;
} kinc_g4_shader_impl_t;

#ifdef __cplusplus
}
#endif
