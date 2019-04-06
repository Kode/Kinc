#include "pch.h"

#include <Kinc/Graphics4/Shader.h>

#include <Kore/Math/Core.h>
#include <Kore/SystemMicrosoft.h>

#include "Direct3D11.h"

#include <stdint.h>

void Kinc_G4_Shader_Destroy(Kinc_G4_Shader *shader) {
	if (shader->impl.shader != nullptr) {
		switch (shader->impl.type) {
		case KINC_SHADER_TYPE_VERTEX:
			((ID3D11VertexShader*)shader)->Release();
			break;
		case KINC_SHADER_TYPE_FRAGMENT:
			((ID3D11PixelShader*)shader)->Release();
			break;
		case KINC_SHADER_TYPE_GEOMETRY:
			((ID3D11GeometryShader*)shader)->Release();
			break;
		case KINC_SHADER_TYPE_TESSELLATION_CONTROL:
			((ID3D11HullShader*)shader)->Release();
			break;
		case KINC_SHADER_TYPE_TESSELLATION_EVALUATION:
			((ID3D11DomainShader*)shader)->Release();
			break;
		}
		free(shader->impl.data);
	}
}

void Kinc_G4_Shader_Create(Kinc_G4_Shader *shader, void *_data, int length, Kinc_G4_ShaderType type) {
	unsigned index = 0;
	uint8_t *data = (uint8_t*)_data;
	shader->impl.type = (int)type;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		attributes[name] = data[index++];
	}

	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		textures[name] = data[index++];
	}

	uint8_t constantCount = data[index++];
	constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		ShaderConstant constant;
		constant.offset = *(u32*)&data[index];
		index += 4;
		constant.size = *(u32*)&data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;
		constants[name] = constant;
		constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = length - index;
	shader->impl.data = (uint8_t*)malloc(shader->impl.length);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	switch (type) {
	case KINC_SHADER_TYPE_VERTEX:
		Kinc_Microsoft_Affirm(device->CreateVertexShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11VertexShader **)&shader->impl.shader));
		break;
	case KINC_SHADER_TYPE_FRAGMENT:
		Kinc_Microsoft_Affirm(device->CreatePixelShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11PixelShader **)&shader->impl.shader));
		break;
	case KINC_SHADER_TYPE_GEOMETRY:
		Kinc_Microsoft_Affirm(device->CreateGeometryShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11GeometryShader **)&shader->impl.shader));
		break;
	case KINC_SHADER_TYPE_TESSELLATION_CONTROL:
		Kinc_Microsoft_Affirm(device->CreateHullShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11HullShader **)&shader->impl.shader));
		break;
	case KINC_SHADER_TYPE_TESSELLATION_EVALUATION:
		Kinc_Microsoft_Affirm(device->CreateDomainShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11DomainShader **)&shader->impl.shader));
		break;
	}
}

#ifdef KRAFIX_LIBRARY
extern void krafix_compile(const char* source, char* output, int* length, const char* targetlang, const char* system, const char* shadertype);
#endif

void Kinc_G4_Shader_CreateFromSource(Kinc_G4_Shader *shader, const char *source, Kinc_G4_ShaderType type) {
#ifdef KRAFIX_LIBRARY
	char* output = new char[1024 * 1024];
	int length;
	krafix_compile(source, output, &length, "d3d11", "windows", type == Graphics4::FragmentShader ? "frag" : "vert");
	parse(output, length, type);
#endif
}
