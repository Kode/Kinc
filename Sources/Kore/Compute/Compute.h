#pragma once

#include <Kore/ComputeImpl.h>
#ifdef KORE_OPENGL
#include <Kore/ShaderStorageBufferImpl.h>
#endif

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

#ifdef KORE_OPENGL
	class ShaderStorageBuffer : public ShaderStorageBufferImpl {
	public:
		ShaderStorageBuffer(int count, Graphics4::VertexData type);
		virtual ~ShaderStorageBuffer();
		int* lock();
		void unlock();
		int count();
		void _set();
	};
#endif

	namespace Compute {
		void setFloat(ComputeConstantLocation location, float value);
#ifdef KORE_OPENGL
		void setBuffer(ShaderStorageBuffer* buffer, int index);
#endif
		void setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
