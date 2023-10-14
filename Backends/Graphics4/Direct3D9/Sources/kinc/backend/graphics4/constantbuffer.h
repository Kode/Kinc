#pragma once

#ifdef KINC_KONG

typedef struct kinc_g4_constant_buffer_impl {
	size_t size;
	size_t last_start;
	size_t last_size;
} kinc_g4_constant_buffer_impl;

#endif
