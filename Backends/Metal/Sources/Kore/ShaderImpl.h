#pragma once

namespace Kore {
	class Program;
	class ProgramImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* source, int length);
		u8* source;
		int length;
		friend class Program;
		friend class ProgramImpl;
	};
}
