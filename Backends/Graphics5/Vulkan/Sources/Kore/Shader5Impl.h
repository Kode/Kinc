#pragma once

namespace Kore {
	namespace Graphics5 {
		class Program;
	}

	class Program5Impl;

	class Shader5Impl {
	public:
		Shader5Impl(void* source, int length);
		virtual ~Shader5Impl();
		uint id;
		char* source;
		int length;
		friend class Program;
		friend class ProgramImpl;
	};
}
