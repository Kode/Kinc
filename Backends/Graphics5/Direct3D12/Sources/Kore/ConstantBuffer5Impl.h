#pragma once

struct ID3D12Resource;

namespace Kore {
	class ConstantBuffer5Impl {
	public:
		ID3D12Resource* _buffer;
	protected:
		int lastStart;
		int lastCount;
		int mySize;
	};
}
