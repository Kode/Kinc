#pragma once

namespace Kore {
	namespace Graphics4 {
		class PipelineState;
	}

	class PipelineStateImpl;

	class ShaderImpl {
	public:
		ShaderImpl(void* data, int length);
		ShaderImpl(const char* source);
		virtual ~ShaderImpl();
		uint _glid;
		const char* source;
		int length;
		friend class Graphics4::PipelineState;
		friend class PipelineStateImpl;
	};

	class ConstantLocationImpl {
	public:
		int location;
		unsigned int type;
	};
}
