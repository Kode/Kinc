#pragma once

#include <objc/runtime.h>

namespace Kore {
	class ComputeConstantLocationImpl {
	public:
		u32 _offset;
	};

	class ComputeTextureUnitImpl {
	public:
		u32 _index;
	};

	class ComputeShaderImpl {
	public:
		ComputeShaderImpl(void* source, int length);
		~ComputeShaderImpl();
		char name[1024];
		id _function;
		id _pipeline;
		id _reflection;
	};
}
