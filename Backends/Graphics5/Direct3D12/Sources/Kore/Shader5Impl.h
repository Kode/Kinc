#pragma once

#include "pch.h"

#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t offset;
	uint32_t size;
} ShaderConstant;

typedef struct {
	std::map<std::string, ShaderConstant> constants;
	int constantsSize;
	std::map<std::string, int> attributes;
	std::map<std::string, int> textures;
	void *shader;
	uint8_t *data;
	int length;
} Shader5Impl;

#ifdef __cplusplus
}
#endif
