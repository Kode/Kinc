#pragma once

struct ID3D12Resource;
struct ID3D12DescriptorHeap;

namespace Kore {
	struct D3D12Viewport {
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};

	struct D3D12Rect {
		long left;
		long top;
		long right;
		long bottom;
	};

	class RenderTargetImpl {
	public:
		ID3D12Resource* renderTarget;
		ID3D12DescriptorHeap* renderTargetDescriptorHeap;
		ID3D12DescriptorHeap* srvDescriptorHeap;
		D3D12Viewport viewport;
		D3D12Rect scissor;
		int stage;
	};
}
