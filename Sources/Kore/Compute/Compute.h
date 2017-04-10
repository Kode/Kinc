#pragma once

#include <Kore/ComputeImpl.h>
#include <Kore/ShaderStorageBufferImpl.h>

namespace Kore {
	namespace Graphics4 {
		class Texture;
	}

	class ComputeConstantLocation : public ComputeConstantLocationImpl {};

	class ComputeTextureUnit : public ComputeTextureUnitImpl {};

	class ComputeShader : public ComputeShaderImpl {
	public:
		ComputeShader(void* source, int length);
		ComputeConstantLocation getConstantLocation(const char* name);
		ComputeTextureUnit getTextureUnit(const char* name);
	};

	class ShaderStorageBuffer : public ShaderStorageBufferImpl {
	public:
		ShaderStorageBuffer(int count, VertexData type);
		virtual ~ShaderStorageBuffer();
		int* lock();
		void unlock();
		int count();
		void _set();
	};

	namespace Compute {
		void setFloat(ComputeConstantLocation location, float value);
		void setBuffer(ShaderStorageBuffer* buffer, int index);
		void setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
