#pragma once

namespace Kore {
	class Program;
	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* source, int length);
		void* mtlFunction;
		friend class Program;
		friend class ProgramImpl;
	};
}
