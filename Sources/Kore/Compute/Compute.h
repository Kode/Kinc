#pragma once

#include <Kore/ComputeImpl.h>
#ifdef KORE_OPENGL
#include <Kore/ShaderStorageBufferImpl.h>
#endif
#include <Kore/Math/Matrix.h>

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
		enum Access { Read, Write, ReadWrite };

		void setBool(ComputeConstantLocation location, bool value);
		void setInt(ComputeConstantLocation location, int value);
		void setFloat(ComputeConstantLocation location, float value);
		void setFloat2(ComputeConstantLocation location, float value1, float value2);
		void setFloat3(ComputeConstantLocation location, float value1, float value2, float value3);
		void setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4);
		void setFloats(ComputeConstantLocation location, float* values, int count);
		void setMatrix(ComputeConstantLocation location, const mat4& value);
		void setMatrix(ComputeConstantLocation location, const mat3& value);
#ifdef KORE_OPENGL
		void setBuffer(ShaderStorageBuffer* buffer, int index);
#endif
		void setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
