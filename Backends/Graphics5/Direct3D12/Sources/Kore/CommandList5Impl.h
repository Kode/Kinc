#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

struct kinc_g5_pipeline;

typedef struct {
	ID3D12CommandAllocator *_commandAllocator;
	ID3D12GraphicsCommandList *_commandList;
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;
	bool closed;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
