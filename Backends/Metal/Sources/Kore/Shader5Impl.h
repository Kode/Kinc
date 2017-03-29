#pragma once

#include <objc/runtime.h>

namespace Kore {
	namespace Graphics5 {
		class Program;
	}
	
	class Program5Impl;

	class Shader5Impl {
	public:
		Shader5Impl(void* source, int length);
		char name[1024];
		id mtlFunction;
		friend class Graphics5::Program;
		friend class Program5Impl;
	};
}
