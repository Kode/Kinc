#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Resource;

struct D3D12IindexBufferView {
	__int64 BufferLocation;
	unsigned int SizeInBytes;
	int Format;
};

typedef struct {
	struct ID3D12Resource *indexBuffer;
	struct D3D12IindexBufferView indexBufferView;
	struct ID3D12Resource *uploadBuffer;
	int myCount;
	//static IndexBuffer5Impl *_current;
	bool _gpuMemory;
} IndexBuffer5Impl;

struct kinc_g5_index_buffer;

void kinc_g5_internal_index_buffer_upload(struct kinc_g5_index_buffer *buffer, struct ID3D12GraphicsCommandList *commandList);

#ifdef __cplusplus
}
#endif
