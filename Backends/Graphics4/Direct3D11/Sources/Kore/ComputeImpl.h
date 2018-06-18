#pragma once

#include <map>
#include <string>

struct ID3D11Buffer;

namespace Kore {
	class ComputeConstantLocationImpl {
	public:
		u32 offset;
		u32 size;
	};

	class ComputeTextureUnitImpl {};

	struct ComputeShaderConstant {
		u32 offset;
		u32 size;
		u8 columns;
		u8 rows;
	};

	class ComputeShaderImpl {
	public:
		ComputeShaderImpl();
		std::map<std::string, ComputeShaderConstant> constants;
		int constantsSize;
		std::map<std::string, int> attributes;
		std::map<std::string, int> textures;
		void* shader;
		u8* data;
		int length;
		ID3D11Buffer* constantBuffer;
	};
}
