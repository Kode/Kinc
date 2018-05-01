#include "pch.h"

#include "Direct3D11.h"
#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/SystemMicrosoft.h>

using namespace Kore;

ShaderImpl::ShaderImpl() {}

void Graphics4::Shader::parse(void* _data, int length, ShaderType type) {
	unsigned index = 0;
	u8* data = (u8*)_data;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		attributes[name] = data[index++];
	}

	u8 texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		textures[name] = data[index++];
	}

	u8 constantCount = data[index++];
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

	this->length = length - index;
	this->data = (u8*)malloc(this->length);
	memcpy(this->data, &data[index], this->length);

	switch (type) {
	case VertexShader:
		Microsoft::affirm(device->CreateVertexShader(this->data, this->length, nullptr, (ID3D11VertexShader**)&shader));
		break;
	case FragmentShader:
		Microsoft::affirm(device->CreatePixelShader(this->data, this->length, nullptr, (ID3D11PixelShader**)&shader));
		break;
	case GeometryShader:
		Microsoft::affirm(device->CreateGeometryShader(this->data, this->length, nullptr, (ID3D11GeometryShader**)&shader));
		break;
	case TessellationControlShader:
		Microsoft::affirm(device->CreateHullShader(this->data, this->length, nullptr, (ID3D11HullShader**)&shader));
		break;
	case TessellationEvaluationShader:
		Microsoft::affirm(device->CreateDomainShader(this->data, this->length, nullptr, (ID3D11DomainShader**)&shader));
		break;
	}
}

Graphics4::Shader::Shader(void* _data, int length, ShaderType type) {
	setId();
	parse(_data, length, type);
}

#ifdef KRAFIX_LIBRARY
extern void krafix_compile(const char* source, char* output, int* length, const char* targetlang, const char* system, const char* shadertype);
#endif

Graphics4::Shader::Shader(const char* source, Graphics4::ShaderType type) {
	setId();
#ifdef KRAFIX_LIBRARY
	char* output = new char[1024 * 1024];
	int length;
	krafix_compile(source, output, &length, "d3d11", "windows", type == Graphics4::FragmentShader ? "frag" : "vert");
	parse(output, length, type);
#endif
}
