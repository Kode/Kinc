#pragma once

#ifdef KINC_KONG

#include <stddef.h>

typedef struct kinc_g4_constant_buffer_impl {
	unsigned buffer;
	void *data;
	size_t size;
	size_t last_start;
	size_t last_size;
} kinc_g4_constant_buffer_impl;

#endif
