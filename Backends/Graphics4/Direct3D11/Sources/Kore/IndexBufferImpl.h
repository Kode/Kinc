#pragma once

struct ID3D11Buffer;

typedef struct {
	struct ID3D11Buffer *ib;
	int *indices;
	int count;
} Kinc_G4_IndexBufferImpl;

//**static Graphics4::IndexBuffer* _current;
