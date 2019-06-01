#pragma once

#include <Kore/ComputeImpl.h>
#ifdef KORE_OPENGL
#include <Kore/ShaderStorageBufferImpl.h>
#endif
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Matrix.h>

namespace Kore {
	namespace Graphics4 {
		class Texture;
		class RenderTarget;
	}

	class ComputeConstantLocation {
	public:
		kinc_compute_constant_location_impl_t kincImpl;
	};

	class ComputeTextureUnit {
	public:
		kinc_compute_texture_unit_impl_t kincImpl;
	};

	class ComputeShader {
	public:
		ComputeShader(void* source, int length);
		ComputeConstantLocation getConstantLocation(const char* name);
		ComputeTextureUnit getTextureUnit(const char* name);
		kinc_compute_shader_impl_t kincImpl;
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
		void setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* texture, Access access);
		void setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture);
		void setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target);
		void setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target);
		void setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing);
		void setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter);
		void setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter);
		void setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter);
		void setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing);
		void setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter);
		void setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter);
		void setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
