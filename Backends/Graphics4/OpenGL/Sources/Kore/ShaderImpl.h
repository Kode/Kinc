#pragma once

namespace Kore {
	namespace Graphics4 {
		class Program;
	}

	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* source, int length);
		virtual ~ShaderImpl();
		uint id;
		char* source;
		int length;
		friend class Graphics4::Program;
		friend class ProgramImpl;
	};
}
