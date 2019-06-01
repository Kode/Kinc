#pragma once

#include <map>
#include <string>

struct ID3D11Buffer;

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_compute_texture_unit_impl_t;

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} kinc_compute_internal_shader_constant_t;

typedef struct {
	std::map<std::string, kinc_compute_internal_shader_constant_t> constants;
	int constantsSize;
	std::map<std::string, int> attributes;
	std::map<std::string, int> textures;
	void* shader;
	uint8_t *data;
	int length;
	ID3D11Buffer* constantBuffer;
} kinc_compute_shader_impl_t;
