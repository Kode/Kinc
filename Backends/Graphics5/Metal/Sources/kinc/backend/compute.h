#pragma once

#include <stdint.h>

typedef struct {
	uint32_t _offset;
} kinc_compute_constant_location_impl_t;

typedef struct {
	uint32_t _index;
} kinc_compute_texture_unit_impl_t;

typedef struct {
	char name[1024];
	void *_function;
	void *_pipeline;
	void *_reflection;
} kinc_compute_shader_impl_t;
