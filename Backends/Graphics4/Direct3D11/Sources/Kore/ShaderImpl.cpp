#include "pch.h"

#include <kinc/graphics4/shader.h>

#include <Kore/Math/Core.h>
#include <Kore/SystemMicrosoft.h>

#include "Direct3D11.h"

#include <assert.h>
#include <stdint.h>

void kinc_g4_shader_destroy(kinc_g4_shader_t *shader) {
	if (shader->impl.shader != nullptr) {
		switch (shader->impl.type) {
		case KINC_G4_SHADER_TYPE_VERTEX:
			((ID3D11VertexShader*)shader)->Release();
			break;
		case KINC_G4_SHADER_TYPE_FRAGMENT:
			((ID3D11PixelShader*)shader)->Release();
			break;
		case KINC_G4_SHADER_TYPE_GEOMETRY:
			((ID3D11GeometryShader*)shader)->Release();
			break;
		case KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL:
			((ID3D11HullShader*)shader)->Release();
			break;
		case KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION:
			((ID3D11DomainShader*)shader)->Release();
			break;
		}
		free(shader->impl.data);
	}
}

// djb2
uint32_t kinc_internal_hash_name(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while (c = *str++) {
		hash = hash * 33 ^ c;
	}
	return hash;
}

void kinc_g4_shader_init(kinc_g4_shader_t *shader, void *_data, size_t length, kinc_g4_shader_type_t type) {
	unsigned index = 0;
	uint8_t *data = (uint8_t*)_data;
	shader->impl.type = (int)type;

	memset(&shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.attributes[i].hash = kinc_internal_hash_name(name);
		shader->impl.attributes[i].index = data[index++];
	}

	memset(&shader->impl.textures, 0, sizeof(shader->impl.textures));
	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.textures[i].hash = kinc_internal_hash_name(name);
		shader->impl.textures[i].index = data[index++];
	}

	memset(&shader->impl.constants, 0, sizeof(shader->impl.constants));
	uint8_t constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		kinc_internal_shader_constant_t constant;
		constant.hash = kinc_internal_hash_name(name);
		constant.offset = *(uint32_t*)&data[index];
		index += 4;
		constant.size = *(uint32_t*)&data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;

		shader->impl.constants[i] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = (int)(length - index);
	shader->impl.data = (uint8_t*)malloc(shader->impl.length);
	assert(shader->impl.data != NULL);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	switch (type) {
	case KINC_G4_SHADER_TYPE_VERTEX:
		kinc_microsoft_affirm(device->CreateVertexShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11VertexShader **)&shader->impl.shader));
		break;
	case KINC_G4_SHADER_TYPE_FRAGMENT:
		kinc_microsoft_affirm(device->CreatePixelShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11PixelShader **)&shader->impl.shader));
		break;
	case KINC_G4_SHADER_TYPE_GEOMETRY:
		kinc_microsoft_affirm(device->CreateGeometryShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11GeometryShader **)&shader->impl.shader));
		break;
	case KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL:
		kinc_microsoft_affirm(device->CreateHullShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11HullShader **)&shader->impl.shader));
		break;
	case KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION:
		kinc_microsoft_affirm(device->CreateDomainShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11DomainShader **)&shader->impl.shader));
		break;
	}
}

#ifdef KRAFIX_LIBRARY
extern void krafix_compile(const char* source, char* output, int* length, const char* targetlang, const char* system, const char* shadertype);
#endif

void kinc_g4_shader_init_from_source(kinc_g4_shader_t *shader, const char *source, kinc_g4_shader_type_t type) {
#ifdef KRAFIX_LIBRARY
	char* output = new char[1024 * 1024];
	int length;
	krafix_compile(source, output, &length, "d3d11", "windows", type == Graphics4::FragmentShader ? "frag" : "vert");
	parse(output, length, type);
#endif
}
