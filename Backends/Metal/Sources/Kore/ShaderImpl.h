#pragma once

#include <objc/runtime.h>

namespace Kore {
	class Program;
	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* source, int length);
		id mtlFunction;
		friend class Program;
		friend class ProgramImpl;
	};
}
