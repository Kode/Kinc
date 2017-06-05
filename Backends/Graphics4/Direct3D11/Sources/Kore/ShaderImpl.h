#pragma once

#include <map>
#include <string>

namespace Kore {
	struct ShaderConstant {
		u32 offset;
		u32 size;
	};

	class ShaderImpl {
	public:
		ShaderImpl();
		std::map<std::string, ShaderConstant> constants;
		int constantsSize;
		std::map<std::string, int> attributes;
		std::map<std::string, int> textures;
		void* shader;
		u8* data;
		int length;
	};

	class ConstantLocationImpl {
	public:
		u8 vertexOffset;
		u8 vertexSize;
		u8 fragmentOffset;
		u8 fragmentSize;
		u8 geometryOffset;
		u8 geometrySize;
		u8 tessEvalOffset;
		u8 tessEvalSize;
		u8 tessControlOffset;
		u8 tessControlSize;
	};
}
