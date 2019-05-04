#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Resource;

typedef struct {
	ID3D12Resource *_buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

const bool kinc_g5_transposeMat3 = false;
const bool kinc_g5_transposeMat4 = false;

#ifdef __cplusplus
}
#endif
