#include "pch.h"

#include "Direct3D11.h"
#include "PipelineStateImpl.h"
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/WinError.h>

using namespace Kore;
using namespace Kore::Graphics4;

namespace Kore {
	PipelineState* currentPipeline = nullptr;
}

void PipelineStateImpl::setConstants() {
	if (currentPipeline->vertexShader->constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->vertexConstantBuffer, 0, nullptr, vertexConstants, 0, 0);
		context->VSSetConstantBuffers(0, 1, &currentPipeline->vertexConstantBuffer);
	}
	if (currentPipeline->fragmentShader->constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->fragmentConstantBuffer, 0, nullptr, fragmentConstants, 0, 0);
		context->PSSetConstantBuffers(0, 1, &currentPipeline->fragmentConstantBuffer);
	}
	if (currentPipeline->geometryShader != nullptr && currentPipeline->geometryShader->constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->geometryConstantBuffer, 0, nullptr, geometryConstants, 0, 0);
		context->GSSetConstantBuffers(0, 1, &currentPipeline->geometryConstantBuffer);
	}
	if (currentPipeline->tessellationControlShader != nullptr && currentPipeline->tessellationControlShader->constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->tessControlConstantBuffer, 0, nullptr, tessControlConstants, 0, 0);
		context->HSSetConstantBuffers(0, 1, &currentPipeline->tessControlConstantBuffer);
	}
	if (currentPipeline->tessellationEvaluationShader != nullptr && currentPipeline->tessellationEvaluationShader->constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->tessEvalConstantBuffer, 0, nullptr, tessEvalConstants, 0, 0);
		context->DSSetConstantBuffers(0, 1, &currentPipeline->tessEvalConstantBuffer);
	}
}

PipelineStateImpl::PipelineStateImpl() {}

void PipelineStateImpl::set(PipelineState* pipeline) {
	currentPipeline = pipeline;
	context->VSSetShader((ID3D11VertexShader*)pipeline->vertexShader->shader, nullptr, 0);
	context->PSSetShader((ID3D11PixelShader*)pipeline->fragmentShader->shader, nullptr, 0);

	if (pipeline->geometryShader != nullptr) context->GSSetShader((ID3D11GeometryShader*)pipeline->geometryShader->shader, nullptr, 0);
	if (pipeline->tessellationControlShader != nullptr) context->HSSetShader((ID3D11HullShader*)pipeline->tessellationControlShader->shader, nullptr, 0);
	if (pipeline->tessellationEvaluationShader != nullptr) context->DSSetShader((ID3D11DomainShader*)pipeline->tessellationEvaluationShader->shader, nullptr, 0);

	context->IASetInputLayout(pipeline->d3d11inputLayout);
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char* name) {
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

	if (tessellationControlShader == nullptr || tessellationControlShader->constants.find(name) == tessellationControlShader->constants.end()) {
		location.tessControlOffset = 0;
		location.tessControlSize = 0;
	}
	else {
		ShaderConstant constant = tessellationControlShader->constants[name];
		location.tessControlOffset = constant.offset;
		location.tessControlSize = constant.size;
	}

	if (tessellationEvaluationShader == nullptr || tessellationEvaluationShader->constants.find(name) == tessellationEvaluationShader->constants.end()) {
		location.tessEvalOffset = 0;
		location.tessEvalSize = 0;
	}
	else {
		ShaderConstant constant = tessellationEvaluationShader->constants[name];
		location.tessEvalOffset = constant.offset;
		location.tessEvalSize = constant.size;
	}

	return location;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char* name) {
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

namespace {
	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	void setVertexDesc(D3D11_INPUT_ELEMENT_DESC& vertexDesc, int attributeIndex, int index, int stream) {
		vertexDesc.SemanticName = "TEXCOORD";
		vertexDesc.SemanticIndex = attributeIndex;
		vertexDesc.InputSlot = stream;
		vertexDesc.AlignedByteOffset = (index == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc.InputSlotClass = stream == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA; // hack
		vertexDesc.InstanceDataStepRate = stream == 0 ? 0 : 1;                                                 // hack
	}
}

void Graphics4::PipelineState::compile() {
	if (vertexShader->constantsSize > 0)
		affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(vertexShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                            &vertexConstantBuffer));
	if (fragmentShader->constantsSize > 0)
		affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(fragmentShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                            &fragmentConstantBuffer));
	if (geometryShader != nullptr && geometryShader->constantsSize > 0)
		affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(geometryShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                            &geometryConstantBuffer));
	if (tessellationControlShader != nullptr && tessellationControlShader->constantsSize > 0)
		affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(tessellationControlShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                            &tessControlConstantBuffer));
	if (tessellationEvaluationShader != nullptr && tessellationEvaluationShader->constantsSize > 0)
		affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(tessellationEvaluationShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                            &tessEvalConstantBuffer));

	int all = 0;
	for (int stream = 0; inputLayout[stream] != nullptr; ++stream) {
		for (int index = 0; index < inputLayout[stream]->size; ++index) {
			if (inputLayout[stream]->elements[index].data == Float4x4VertexData) {
				all += 4;
			}
			else {
				all += 1;
			}
		}
	}

	D3D11_INPUT_ELEMENT_DESC* vertexDesc = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * all);
	int i = 0;
	for (int stream = 0; inputLayout[stream] != nullptr; ++stream) {
		for (int index = 0; index < inputLayout[stream]->size; ++index) {
			switch (inputLayout[stream]->elements[index].data) {
			case Float1VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[inputLayout[stream]->elements[index].name], index, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
				++i;
				break;
			case Float2VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[inputLayout[stream]->elements[index].name], index, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				++i;
				break;
			case Float3VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[inputLayout[stream]->elements[index].name], index, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				++i;
				break;
			case Float4VertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[inputLayout[stream]->elements[index].name], index, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				++i;
				break;
			case ColorVertexData:
				setVertexDesc(vertexDesc[i], vertexShader->attributes[inputLayout[stream]->elements[index].name], index, stream);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
				++i;
				break;
			case Float4x4VertexData:
				for (int i2 = 0; i2 < 4; ++i2) {
					char name[101];
					strcpy(name, inputLayout[stream]->elements[index].name);
					strcat(name, "_");
					size_t length = strlen(name);
					_itoa(i2, &name[length], 10);
					name[length + 1] = 0;

					setVertexDesc(vertexDesc[i], vertexShader->attributes[name], index + i2, stream);
					vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

					++i;
				}
				break;
			}
		}
	}

	affirm(device->CreateInputLayout(vertexDesc, all, vertexShader->data, vertexShader->length, &d3d11inputLayout));
}
