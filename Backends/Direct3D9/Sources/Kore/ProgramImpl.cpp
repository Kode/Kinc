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

void Program::setTesselationEvaluationShader(Shader* shader) {
	log(Error, "Direct3D 9 does not support tesselation shaders.");
}

void Program::setTesselationControlShader(Shader* shader) {
	log(Error, "Direct3D 9 does not support tesselation shaders.");
}

void Program::link(const VertexStructure& structure) {
	int stride = 0;
	D3DVERTEXELEMENT9* elements = new D3DVERTEXELEMENT9[structure.size + 1];
	for (int i = 0; i < structure.size; ++i) {
		elements[i].Stream = 0;
		elements[i].Offset = stride;
		switch (structure.elements[i].data) {
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
		}
		elements[i].Method = D3DDECLMETHOD_DEFAULT;
		elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
		elements[i].UsageIndex = vertexShader->attributes[structure.elements[i].name];
	}
	elements[structure.size].Stream = 0xff;
	elements[structure.size].Offset = 0;
	elements[structure.size].Type = D3DDECLTYPE_UNUSED;
	elements[structure.size].Method = 0;
	elements[structure.size].Usage = 0;
	elements[structure.size].UsageIndex = 0;

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
	}
	return unit;
}
