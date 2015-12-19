#include "pch.h"
#include "ProgramImpl.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Application.h>
#include <Kore/Log.h>
#include <Kore/WinError.h>
#include "Direct3D9.h"

using namespace Kore;

Program::Program() {
	vertexShader = nullptr;
	fragmentShader = nullptr;
}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

void Program::setGeometryShader(Shader* shader) {
	log(Error, "Direct3D 9 does not support geometry shaders.");
}

void Program::setTesselationControlShader(Shader* shader) {
	log(Error, "Direct3D 9 does not support tesselation shaders.");
}

void Program::setTesselationEvaluationShader(Shader* shader) {
	log(Error, "Direct3D 9 does not support tesselation shaders.");
}

void Program::link(VertexStructure** structures, int count) {
	if (count > 1) {
		int a = 3;
		++a;
	}
	int all = 0;
	for (int stream = 0; stream < count; ++stream) {
		for (int index = 0; index < structures[stream]->size; ++index) {
			if (structures[stream]->elements[index].data == Float4x4VertexData) {
				all += 4;
			}
			else {
				all += 1;
			}
		}
	}
	
	D3DVERTEXELEMENT9* elements = (D3DVERTEXELEMENT9*)alloca(sizeof(D3DVERTEXELEMENT9) * (all + 1));
	int i = 0;
	for (int stream = 0; stream < count; ++stream) {
		int stride = 0;
		for (int index = 0; index < structures[stream]->size; ++index) {
			if (structures[stream]->elements[index].data != Float4x4VertexData) {
				elements[i].Stream = stream;
				elements[i].Offset = stride;
			}
			switch (structures[stream]->elements[index].data) {
			case Float1VertexData:
				elements[i].Type = D3DDECLTYPE_FLOAT1;
				stride += 4 * 1;
				break;
			case Float2VertexData:
				elements[i].Type = D3DDECLTYPE_FLOAT2;
				stride += 4 * 2;
				break;
			case Float3VertexData:
				elements[i].Type = D3DDECLTYPE_FLOAT3;
				stride += 4 * 3;
				break;
			case Float4VertexData:
				elements[i].Type = D3DDECLTYPE_FLOAT4;
				stride += 4 * 4;
				break;
			case ColorVertexData:
				elements[i].Type = D3DDECLTYPE_D3DCOLOR;
				stride += 4;
				break;
			case Float4x4VertexData:
				for (int i2 = 0; i2 < 4; ++i2) {
					elements[i].Stream = stream;
					elements[i].Offset = stride;
					elements[i].Type = D3DDECLTYPE_FLOAT4;
					elements[i].Method = D3DDECLMETHOD_DEFAULT;
					elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
					char name[101];
					strcpy(name, structures[stream]->elements[index].name);
					strcat(name, "_");
					size_t length = strlen(name);
					_itoa(i2, &name[length], 10);
					name[length + 1] = 0;
					elements[i].UsageIndex = vertexShader->attributes[name];
					stride += 4 * 4;
					++i;
				}
				break;
			}
			if (structures[stream]->elements[index].data != Float4x4VertexData) {
				elements[i].Method = D3DDECLMETHOD_DEFAULT;
				elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
				elements[i].UsageIndex = vertexShader->attributes[structures[stream]->elements[index].name];
				++i;
			}
		}
	}
	elements[all].Stream = 0xff;
	elements[all].Offset = 0;
	elements[all].Type = D3DDECLTYPE_UNUSED;
	elements[all].Method = 0;
	elements[all].Usage = 0;
	elements[all].UsageIndex = 0;

	vertexDecleration = nullptr;
	affirm(device->CreateVertexDeclaration(elements, &vertexDecleration));

	if (vertexShader->constants.find("dx_ViewAdjust") != vertexShader->constants.end()) halfPixelLocation = vertexShader->constants["dx_ViewAdjust"].regindex;
	else halfPixelLocation = vertexShader->constants["dx_HalfPixelSize"].regindex;
}

void Program::set() {
	affirm(device->SetVertexShader((IDirect3DVertexShader9*)vertexShader->shader));
	affirm(device->SetPixelShader((IDirect3DPixelShader9*)fragmentShader->shader));
	affirm(device->SetVertexDeclaration(vertexDecleration));
	
	float floats[4];
	floats[0] = 1.0f / Application::the()->width();
	floats[1] = 1.0f / Application::the()->height();
	floats[2] = floats[0];
	floats[3] = floats[1];
	affirm(device->SetVertexShaderConstantF(halfPixelLocation, floats, 1));
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	char d3dname[101];
	strcpy(d3dname, "_");
	strcat(d3dname, name);

	if (fragmentShader->constants.find(d3dname) != fragmentShader->constants.end()) {
		location.reg = fragmentShader->constants[d3dname];
		location.shaderType = 1;
	}
	else if (vertexShader->constants.find(d3dname) != vertexShader->constants.end()) {
		location.reg = vertexShader->constants[d3dname];
		location.shaderType = 0;
	}
	else {
		location.shaderType = -1;
		log(Warning, "Could not find uniform %s.", name);
	}
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	char d3dname[101];
	strcpy(d3dname, "_");
	strcat(d3dname, name);

	if (fragmentShader->constants.find(d3dname) != fragmentShader->constants.end()) {
		unit.unit = fragmentShader->constants[d3dname].regindex;
	}
	else if (vertexShader->constants.find(d3dname) != vertexShader->constants.end()) {
		unit.unit = vertexShader->constants[d3dname].regindex;
	}
	else {
		unit.unit = -1;
		log(Warning, "Could not find texture %s.", name);
	}
	return unit;
}
