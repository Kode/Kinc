#pragma once

#include <Kore/Graphics4/VertexStructure.h>

namespace Kore {
	class ShaderStorageBuffer;

	class ShaderStorageBufferImpl {
	protected:
	public:
		ShaderStorageBufferImpl(int count, Graphics4::VertexData type);
		void unset();

		int* data;
		int myCount;
		int myStride;
		uint bufferId;
	public:
		static ShaderStorageBuffer* current;
	};
}
