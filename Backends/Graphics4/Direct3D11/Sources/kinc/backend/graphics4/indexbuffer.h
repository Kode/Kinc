#pragma once

struct ID3D11Buffer;

typedef struct {
	struct ID3D11Buffer *ib;
	void *indices;
	int count;
	int usage;
	bool sixteen;
} kinc_g4_index_buffer_impl_t;
