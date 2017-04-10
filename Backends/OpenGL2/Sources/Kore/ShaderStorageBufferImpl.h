#pragma once

#include <Kore/Graphics/VertexStructure.h>

namespace Kore {
	class ShaderStorageBuffer;

	class ShaderStorageBufferImpl {
	protected:
	public:
		ShaderStorageBufferImpl(int count, VertexData type);
		void unset();

		int* data;
		int myCount;
		int myStride;
		uint bufferId;
	public:
		static ShaderStorageBuffer* current;
	};
}
