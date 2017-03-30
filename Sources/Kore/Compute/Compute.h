#pragma once

#include <Kore/ComputeImpl.h>

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

	namespace Compute {
		void setFloat(ComputeConstantLocation location, float value);
		void setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture);
		void setShader(ComputeShader* shader);
		void compute(int x, int y, int z);
	};
}
