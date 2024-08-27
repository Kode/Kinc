#ifndef KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER
#define KOPE_D3D12_COMMANDLIST_STRUCTS_HEADER

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kope_d3d12_command_list {
	struct ID3D12CommandAllocator *allocator;
	struct ID3D12GraphicsCommandList *list;
} kope_d3d12_command_list;

#ifdef __cplusplus
}
#endif

#endif
