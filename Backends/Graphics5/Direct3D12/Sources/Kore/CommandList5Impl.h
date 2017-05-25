#pragma once

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

class CommandList5Impl {
public:
	ID3D12CommandAllocator* _commandAllocator;
	ID3D12GraphicsCommandList* _commandList;
	int _indexCount;
};
