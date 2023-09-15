#pragma once

#ifdef KINC_KONG

struct ID3D11Buffer;

typedef struct kinc_g4_constant_buffer_impl {
	struct ID3D11Buffer *buffer;
	size_t size;
	size_t last_start;
	size_t last_size;
} kinc_g4_constant_buffer_impl;

#endif
