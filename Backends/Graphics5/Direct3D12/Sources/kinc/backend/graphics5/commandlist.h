#pragma once

#include <stdint.h>

#include "d3d12mini.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_pipeline;

typedef struct {
	struct ID3D12CommandAllocator *_commandAllocator;
	struct ID3D12GraphicsCommandList *_commandList;
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;

#ifndef NDEBUG
	bool open;
#endif

	struct D3D12Rect current_full_scissor;

	// keep track of when a command-list is done
	uint64_t fence_value;
	struct ID3D12Fence *fence;
	HANDLE fence_event;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
