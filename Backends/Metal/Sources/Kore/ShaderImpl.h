#pragma once

#include <objc/runtime.h>

namespace Kore {
	class Program;
	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* source, int length);
		char name[1024];
		id mtlFunction;
		friend class Program;
		friend class ProgramImpl;
	};
}
