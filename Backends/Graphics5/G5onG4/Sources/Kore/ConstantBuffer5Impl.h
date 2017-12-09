#pragma once

namespace Kore {
	class ConstantBuffer5Impl {
	public:
		ConstantBuffer5Impl();
	protected:
		int lastStart;
		int lastCount;
		int mySize;
		const bool transposeMat3;
		const bool transposeMat4;
	};
}
