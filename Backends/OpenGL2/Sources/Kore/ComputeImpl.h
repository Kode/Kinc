#pragma once

namespace Kore {
	class ComputeShaderImpl {
	public:
		ComputeShaderImpl(void* source, int length);
		virtual ~ComputeShaderImpl();
		uint _id;
		uint _programid;
		char* _source;
		int _length;
	};
}
