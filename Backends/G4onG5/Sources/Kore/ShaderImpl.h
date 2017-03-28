#pragma once

#include <map>
#include <string>

namespace Kore {
	struct ShaderConstant {
		u8 offset;
		u8 size;
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
}
