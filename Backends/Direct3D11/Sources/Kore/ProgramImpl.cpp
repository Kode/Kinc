#include "pch.h"
#include "ProgramImpl.h"
#include <Kore/Graphics/Shader.h>
#include "Direct3D11.h"

using namespace Kore;

namespace {
	void affirm(HRESULT) { }

	ProgramImpl* current = nullptr;
}

void ProgramImpl::setConstants() {
	if (current->vertexShader->constantsSize > 0) {
		context->UpdateSubresource(current->vertexConstantBuffer, 0, nullptr, vertexConstants, 0, 0);
		context->VSSetConstantBuffers(0, 1, &current->vertexConstantBuffer);
	}
	if (current->fragmentShader->constantsSize > 0) {
		context->UpdateSubresource(current->fragmentConstantBuffer, 0, nullptr, fragmentConstants, 0, 0);
		context->PSSetConstantBuffers(0, 1, &current->fragmentConstantBuffer);
	}
}

Program::Program() {

}

void Program::set() {
	current = this;
	context->VSSetShader((ID3D11VertexShader*)vertexShader->shader, nullptr, 0);
	context->PSSetShader((ID3D11PixelShader*)fragmentShader->shader, nullptr, 0);
	context->IASetInputLayout(inputLayout);
}

ConstantLocation Program::getConstantLocation(const char* name) {
	char d3dname[101];
	strcpy(d3dname, "_");
	strcat(d3dname, name);
	ConstantLocation location;
	if (vertexShader->constants.find(d3dname) == vertexShader->constants.end()) {
		if (fragmentShader->constants.find(d3dname) == fragmentShader->constants.end()) {
			location.vertex = false;
			location.offset = 0;
			location.size = 0;
		}
		else {
			location.vertex = false;
			ShaderConstant constant = fragmentShader->constants[d3dname];
			location.offset = constant.offset;
			location.size = constant.size;
		}
	}
	else {
		location.vertex = true;
		ShaderConstant constant = vertexShader->constants[d3dname];
		location.offset = constant.offset;
		location.size = constant.size;
	}
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	char d3dname[101];
	strcpy(d3dname, "_");
	strcat(d3dname, name);
	TextureUnit unit;
	if (vertexShader->textures.find(d3dname) == vertexShader->textures.end()) {
		if (fragmentShader->textures.find(d3dname) == fragmentShader->textures.end()) {
			unit.unit = -1;
		}
		else {
			unit.unit = fragmentShader->textures[d3dname];
		}
	}
	else {
		unit.unit = vertexShader->textures[d3dname];
	}
	return unit;
}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

namespace {
	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}
}

void Program::link(const VertexStructure& structure) {
	if (vertexShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(vertexShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &vertexConstantBuffer));
	if (fragmentShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(fragmentShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &fragmentConstantBuffer));

	D3D11_INPUT_ELEMENT_DESC vertexDesc[10];
	for (int i = 0; i < structure.size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = vertexShader->attributes[structure.elements[i].name];
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (structure.elements[i].data) {
		case Float2VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case Float3VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case Float4VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case ColorVertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		}
	}

	affirm(device->CreateInputLayout(vertexDesc, structure.size, vertexShader->data, vertexShader->length, &inputLayout));
}
