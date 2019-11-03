#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ShaderRegister {
	uint8_t regtype;
	uint8_t regindex;
	uint8_t regcount;
};

typedef struct {
	struct ShaderRegister reg;
	int shaderType; // 0: Vertex, 1: Fragment
} kinc_g4_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_g4_texture_unit_impl_t;

// class AttributeLocationImpl {};

typedef struct {
	// std::map<std::string, ShaderRegister> constants;
	// std::map<std::string, int> attributes;
	void *shader;
} kinc_g4_shader_impl_t;

#ifdef __cplusplus
}
#endif
