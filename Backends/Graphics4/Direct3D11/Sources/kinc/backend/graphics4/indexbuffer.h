#pragma once

struct ID3D11Buffer;

typedef struct {
	struct ID3D11Buffer *ib;
	int *indices;
	int count;
	int usage;
} kinc_g4_index_buffer_impl_t;
