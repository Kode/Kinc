#pragma once

namespace Kore {
	namespace Graphics4 {
		class Program;
	}

	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* data, int length);
		ShaderImpl(const char* source);
		virtual ~ShaderImpl();
		uint id;
		const char* source;
		int length;
		friend class Graphics4::Program;
		friend class ProgramImpl;
	};
}
