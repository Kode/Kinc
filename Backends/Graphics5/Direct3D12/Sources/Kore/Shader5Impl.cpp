#include "pch.h"

#include "Direct3D12.h"

#include <Kinc/Graphics5/Shader.h>
#include <Kinc/Math/Core.h>
#include <Kore/SystemMicrosoft.h>

void kinc_g5_shader_init(kinc_g5_shader_t *shader, void* _data, int length, kinc_g5_shader_type_t type) {
	memset(shader->impl.constants, 0, sizeof(shader->impl.constants));
	memset(shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	memset(shader->impl.textures, 0, sizeof(shader->impl.textures));

	unsigned index = 0;
	uint8_t* data = (uint8_t*)_data;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		strcpy(shader->impl.attributes[i].name, name);
		shader->impl.attributes->attribute = data[index++];
	}

	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		strcpy(shader->impl.textures[i].name, name);
		shader->impl.textures[i].texture = data[index++];
	}

	uint8_t constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		ShaderConstant constant;
		constant.offset = *(uint32_t*)&data[index];
		index += 4;
		constant.size = *(uint32_t*)&data[index];
		index += 4;
#ifdef KORE_WINDOWS
		index += 2; // columns and rows
#endif
		strcpy(constant.name, name);
		shader->impl.constants[i] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = length - index;
	shader->impl.data = new uint8_t[shader->impl.length];
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	switch (type) {
	case KINC_G5_SHADER_TYPE_VERTEX:
		// Microsoft::affirm(device->CreateVertexShader(this->data, this->length, nullptr, (ID3D11VertexShader**)&shader));
		break;
	case KINC_G5_SHADER_TYPE_FRAGMENT:
		// Microsoft::affirm(device->CreatePixelShader(this->data, this->length, nullptr, (ID3D11PixelShader**)&shader));
		break;
	case KINC_G5_SHADER_TYPE_GEOMETRY:
		// Microsoft::affirm(device->CreateGeometryShader(this->data, this->length, nullptr, (ID3D11GeometryShader**)&shader));
		break;
	case KINC_G5_SHADER_TYPE_TESSELLATION_CONTROL:
		// Microsoft::affirm(device->CreateHullShader(this->data, this->length, nullptr, (ID3D11HullShader**)&shader));
		break;
	case KINC_G5_SHADER_TYPE_TESSELLATION_EVALUATION:
		// Microsoft::affirm(device->CreateDomainShader(this->data, this->length, nullptr, (ID3D11DomainShader**)&shader));
		break;
	}
}

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {}
