#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/WinError.h>
#include "Direct3D11.h"

using namespace Kore;

ShaderImpl::ShaderImpl() {

}

Shader::Shader(void* _data, int length, ShaderType type) {
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
		constant.offset = data[index++];
		constant.size = data[index++];
		constants[name] = constant;
		constantsSize = constant.offset + constant.size;
	}
	
	this->data = &data[index];
	this->length = length - index;

	switch (type) {
	case VertexShader:
		affirm(device->CreateVertexShader(this->data, this->length, nullptr, (ID3D11VertexShader**)&shader));
		break;
	case FragmentShader:
		affirm(device->CreatePixelShader(this->data, this->length, nullptr, (ID3D11PixelShader**)&shader));
		break;
	case GeometryShader:
		affirm(device->CreateGeometryShader(this->data, this->length, nullptr, (ID3D11GeometryShader**)&shader));
		break;
	case TesselationControlShader:
		affirm(device->CreateHullShader(this->data, this->length, nullptr, (ID3D11HullShader**)&shader));
		break;
	case TesselationEvaluationShader:
		affirm(device->CreateDomainShader(this->data, this->length, nullptr, (ID3D11DomainShader**)&shader));
		break;
	}	
}
