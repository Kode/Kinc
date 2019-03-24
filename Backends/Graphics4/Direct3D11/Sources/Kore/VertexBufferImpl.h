#pragma once

struct ID3D11Buffer;

typedef struct {
	struct ID3D11Buffer *vb;
	int stride;
	int count;
	int lockStart;
	int lockCount;
	float* vertices;
	int usage;
		
} Kinc_G4_VertexBufferImpl;
