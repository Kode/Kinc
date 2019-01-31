#pragma once

#include <map>
#include <string>

namespace Kore {
	struct ShaderConstant {
		u32 offset;
		u32 size;
		u8 columns;
		u8 rows;
	};

	class ShaderImpl {
	public:
		ShaderImpl();
		~ShaderImpl();
		std::map<std::string, ShaderConstant> constants;
		int constantsSize;
		std::map<std::string, int> attributes;
		std::map<std::string, int> textures;
		void* shader;
		u8* data;
		int length;
		int type;
	};

	class ConstantLocationImpl {
	public:
		u32 vertexOffset;
		u32 vertexSize;
		u32 fragmentOffset;
		u32 fragmentSize;
		u32 geometryOffset;
		u32 geometrySize;
		u32 tessEvalOffset;
		u32 tessEvalSize;
		u32 tessControlOffset;
		u32 tessControlSize;
		u8 vertexColumns;
		u8 vertexRows;
		u8 fragmentColumns;
		u8 fragmentRows;
		u8 geometryColumns;
		u8 geometryRows;
		u8 tessEvalColumns;
		u8 tessEvalRows;
		u8 tessControlColumns;
		u8 tessControlRows;
	};
}
