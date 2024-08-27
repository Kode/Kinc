#ifndef KOPE_D3D12_BUFFER_STRUCTS_HEADER
#define KOPE_D3D12_BUFFER_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_buffer {
	struct ID3D12Resource *buffer;
} kope_d3d12_buffer;

#ifdef __cplusplus
}
#endif

#endif
