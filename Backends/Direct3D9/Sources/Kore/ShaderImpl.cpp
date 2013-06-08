#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include "Direct3D9.h"
#include <cstdio>

#undef min
#undef max

using namespace Kore;

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

	u8 constantCount = data[index++];
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		ShaderRegister reg;
		reg.regtype = data[index++];
		reg.regindex = data[index++];
		reg.regcount = data[index++];
		constants[name] = reg;
	}
	HRESULT hr;
	if (type == VertexShader)
		hr = device->CreateVertexShader((DWORD*)&data[index], (IDirect3DVertexShader9**)&shader);
	else
		hr = device->CreatePixelShader((DWORD*)&data[index], (IDirect3DPixelShader9**)&shader);
	//if (FAILED(hr)) throw Exception("CreateShader failed");
}
