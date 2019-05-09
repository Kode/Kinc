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
	ID3D12Resource *indexBuffer;
	D3D12IindexBufferView indexBufferView;
	ID3D12Resource *uploadBuffer;
	int myCount;
	//static IndexBuffer5Impl *_current;
	bool _gpuMemory;
} IndexBuffer5Impl;

//void kinc_g5_internal_upload_index_buffer(ID3D12GraphicsCommandList *commandList);

#ifdef __cplusplus
}
#endif
