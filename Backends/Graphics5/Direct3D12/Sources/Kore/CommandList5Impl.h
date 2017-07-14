#pragma once

struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;

namespace Kore {
	namespace Graphics5 {
		class PipelineState;
	}
}

class CommandList5Impl {
public:
	ID3D12CommandAllocator* _commandAllocator;
	ID3D12GraphicsCommandList* _commandList;
	Kore::Graphics5::PipelineState* _currentPipeline;
	int _indexCount;
protected:
	bool closed;
};
