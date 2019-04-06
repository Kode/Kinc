#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t offset;
	uint32_t size;
	uint8_t columns;
	uint8_t rows;
} Kinc_G4_ShaderConstant;

typedef struct {
	// ShaderImpl();
	//~ShaderImpl();
	// std::map<std::string, Kinc_G4_ShaderConstant> constants;
	int constantsSize;
	// std::map<std::string, int> attributes;
	// std::map<std::string, int> textures;
	void *shader;
	uint8_t *data;
	int length;
	int type;
} Kinc_G4_ShaderImpl;

typedef struct {
public:
	uint32_t vertexOffset;
	uint32_t vertexSize;
	uint32_t fragmentOffset;
	uint32_t fragmentSize;
	uint32_t geometryOffset;
	uint32_t geometrySize;
	uint32_t tessEvalOffset;
	uint32_t tessEvalSize;
	uint32_t tessControlOffset;
	uint32_t tessControlSize;
	uint8_t vertexColumns;
	uint8_t vertexRows;
	uint8_t fragmentColumns;
	uint8_t fragmentRows;
	uint8_t geometryColumns;
	uint8_t geometryRows;
	uint8_t tessEvalColumns;
	uint8_t tessEvalRows;
	uint8_t tessControlColumns;
	uint8_t tessControlRows;
} Kinc_G4_ConstantLocationImpl;

typedef struct {
	int unit;
	bool vertex;
} Kinc_G4_TextureUnitImpl;

#ifdef __cplusplus
}
#endif
