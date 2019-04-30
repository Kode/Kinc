#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;

extern const bool kinc_g5_internal_transposeMat3;
extern const bool kinc_g5_internal_transposeMat4;

#ifdef __cplusplus
}
#endif
