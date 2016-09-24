#include "pch.h"
#include "ProgramImpl.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/WinError.h>
#include "Direct3D11.h"

using namespace Kore;

namespace Kore {
	ProgramImpl* currentProgram = nullptr;
}

void ProgramImpl::setConstants() {
	if (currentProgram->vertexShader->constantsSize > 0) {
		context->UpdateSubresource(currentProgram->vertexConstantBuffer, 0, nullptr, vertexConstants, 0, 0);
		context->VSSetConstantBuffers(0, 1, &currentProgram->vertexConstantBuffer);
	}
	if (currentProgram->fragmentShader->constantsSize > 0) {
		context->UpdateSubresource(currentProgram->fragmentConstantBuffer, 0, nullptr, fragmentConstants, 0, 0);
		context->PSSetConstantBuffers(0, 1, &currentProgram->fragmentConstantBuffer);
	}
	if (currentProgram->geometryShader != nullptr && currentProgram->geometryShader->constantsSize > 0) {
		context->UpdateSubresource(currentProgram->geometryConstantBuffer, 0, nullptr, geometryConstants, 0, 0);
		context->GSSetConstantBuffers(0, 1, &currentProgram->geometryConstantBuffer);
	}
	if (currentProgram->tessControlShader != nullptr && currentProgram->tessControlShader->constantsSize > 0) {
		context->UpdateSubresource(currentProgram->tessControlConstantBuffer, 0, nullptr, tessControlConstants, 0, 0);
		context->HSSetConstantBuffers(0, 1, &currentProgram->tessControlConstantBuffer);
	}
	if (currentProgram->tessEvalShader != nullptr && currentProgram->tessEvalShader->constantsSize > 0) {
		context->UpdateSubresource(currentProgram->tessEvalConstantBuffer, 0, nullptr, tessEvalConstants, 0, 0);
		context->DSSetConstantBuffers(0, 1, &currentProgram->tessEvalConstantBuffer);
	}
	
}

ProgramImpl::ProgramImpl() : vertexShader(nullptr), fragmentShader(nullptr), geometryShader(nullptr), tessEvalShader(nullptr), tessControlShader(nullptr) {

}

Program::Program() {

}

void Program::set() {
	currentProgram = this;
	context->VSSetShader((ID3D11VertexShader*)vertexShader->shader, nullptr, 0);
	context->PSSetShader((ID3D11PixelShader*)fragmentShader->shader, nullptr, 0);

	if (geometryShader != nullptr) context->GSSetShader((ID3D11GeometryShader*)geometryShader->shader, nullptr, 0);
	if (tessControlShader != nullptr) context->HSSetShader((ID3D11HullShader*)tessControlShader->shader, nullptr, 0);
	if (tessEvalShader != nullptr) context->DSSetShader((ID3D11DomainShader*)tessEvalShader->shader, nullptr, 0);	

	context->IASetInputLayout(inputLayout);
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;

	if (vertexShader->constants.find(name) == vertexShader->constants.end()) {
		location.vertexOffset = 0;
		location.vertexSize = 0;
	}
	else {
		ShaderConstant constant = vertexShader->constants[name];
		location.vertexOffset = constant.offset;
		location.vertexSize = constant.size;
	}

	if (fragmentShader->constants.find(name) == fragmentShader->constants.end()) {
		location.fragmentOffset = 0;
		location.fragmentSize = 0;
	}
	else {
		ShaderConstant constant = fragmentShader->constants[name];
		location.fragmentOffset = constant.offset;
		location.fragmentSize = constant.size;
	}

	if (geometryShader == nullptr || geometryShader->constants.find(name) == geometryShader->constants.end()) {
		location.geometryOffset = 0;
		location.geometrySize = 0;
	}
	else {
		ShaderConstant constant = geometryShader->constants[name];
		location.geometryOffset = constant.offset;
		location.geometrySize = constant.size;
	}

	if (tessControlShader == nullptr || tessControlShader->constants.find(name) == tessControlShader->constants.end()) {
		location.tessControlOffset = 0;
		location.tessControlSize = 0;
	}
	else {
		ShaderConstant constant = tessControlShader->constants[name];
		location.tessControlOffset = constant.offset;
		location.tessControlSize = constant.size;
	}

	if (tessEvalShader == nullptr || tessEvalShader->constants.find(name) == tessEvalShader->constants.end()) {
		location.tessEvalOffset = 0;
		location.tessEvalSize = 0;
	}
	else {
		ShaderConstant constant = tessEvalShader->constants[name];
		location.tessEvalOffset = constant.offset;
		location.tessEvalSize = constant.size;
	}

	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	if (vertexShader->textures.find(name) == vertexShader->textures.end()) {
		if (fragmentShader->textures.find(name) == fragmentShader->textures.end()) {
			unit.unit = -1;
		}
		else {
			unit.unit = fragmentShader->textures[name];
		}
	}
	else {
		unit.unit = vertexShader->textures[name];
	}
	return unit;
}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

void Program::setGeometryShader(Shader* shader) {
	geometryShader = shader;
}

void Program::setTesselationControlShader(Shader* shader) {
	tessControlShader = shader;
}

void Program::setTesselationEvaluationShader(Shader* shader) {
	tessEvalShader = shader;
}

namespace {
	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	void setVertexDesc(D3D11_INPUT_ELEMENT_DESC& vertexDesc, int index, int i, int stream) {
		vertexDesc.SemanticName = "TEXCOORD";
		vertexDesc.SemanticIndex = index;
		vertexDesc.InputSlot = 0;
		vertexDesc.AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc.InputSlotClass = stream == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA; // hack
		vertexDesc.InstanceDataStepRate = stream == 0 ? 0 : 1; // hack
	}
}

void Program::link(VertexStructure** structures, int count) {
	if (vertexShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(vertexShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &vertexConstantBuffer));
	if (fragmentShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(fragmentShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &fragmentConstantBuffer));
	if (geometryShader != nullptr && geometryShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(geometryShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &geometryConstantBuffer));
	if (tessControlShader != nullptr && tessControlShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(tessControlShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &tessControlConstantBuffer));
	if (tessEvalShader != nullptr && tessEvalShader->constantsSize > 0) affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(tessEvalShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &tessEvalConstantBuffer));

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

	D3D11_INPUT_ELEMENT_DESC* vertexDesc = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * all);
	int i = 0;
	for (int stream = 0; stream < count; ++stream) {
		for (int index = 0; index < structures[stream]->size; ++index) {
			switch (structures[stream]->elements[index].data) {
			case Float1VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[structures[stream]->elements[i].name], i, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
				++i;
				break;
			case Float2VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[structures[stream]->elements[i].name], i, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				++i;
				break;
			case Float3VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[structures[stream]->elements[i].name], i, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				++i;
				break;
			case Float4VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[structures[stream]->elements[i].name], i, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				++i;
				break;
			case ColorVertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[structures[stream]->elements[i].name], i, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
				++i;
				break;
			case Float4x4VertexData:
				for (int i2 = 0; i2 < 4; ++i2) {
					char name[101];
					strcpy(name, structures[stream]->elements[i].name);
					strcat(name, "_");
					size_t length = strlen(name);
					_itoa(i2, &name[length], 10);
					name[length + 1] = 0;

					setVertexDesc(vertexDesc[i], vertexShader->attributes[name], i, stream);
					vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

					++i;
				}
				break;
			}
		}
	}

	affirm(device->CreateInputLayout(vertexDesc, all, vertexShader->data, vertexShader->length, &inputLayout));
}
