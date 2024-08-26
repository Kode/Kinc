#ifndef KOPE_D3D12_DEVICE_STRUCTS_HEADER
#define KOPE_D3D12_DEVICE_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_device {
	struct ID3D12Device *device;
} kope_d3d12_device;

#ifdef __cplusplus
}
#endif

#endif
