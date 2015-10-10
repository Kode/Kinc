#include "pch.h"
#include "ProgramImpl.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/WinError.h>
#include "Direct3D12.h"
#include "d3dx12.h"

using namespace Kore;

ProgramImpl* ProgramImpl::_current = nullptr;

void ProgramImpl::setConstants() {
	/*if (currentProgram->vertexShader->constantsSize > 0) {
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
	*/

	u8* data;
	constantBuffers[currentBackBuffer]->Map(0, nullptr, (void**)&data);
	memcpy(data, vertexConstants, sizeof(vertexConstants));
	memcpy(data + sizeof(vertexConstants), fragmentConstants, sizeof(fragmentConstants));
	constantBuffers[currentBackBuffer]->Unmap(0, nullptr);

	commandList->SetPipelineState(_current->pso);
	commandList->SetGraphicsRootSignature(rootSignature);

	TextureImpl::setTextures();

	commandList->SetGraphicsRootConstantBufferView(1, constantBuffers[currentBackBuffer]->GetGPUVirtualAddress());
}

ProgramImpl::ProgramImpl() : vertexShader(nullptr), fragmentShader(nullptr), geometryShader(nullptr), tessEvalShader(nullptr), tessControlShader(nullptr) {

}

Program::Program() {

}

void Program::set() {
	_current = this;
	//context->VSSetShader((ID3D11VertexShader*)vertexShader->shader, nullptr, 0);
	//context->PSSetShader((ID3D11PixelShader*)fragmentShader->shader, nullptr, 0);

	//if (geometryShader != nullptr) context->GSSetShader((ID3D11GeometryShader*)geometryShader->shader, nullptr, 0);
	//if (tessControlShader != nullptr) context->HSSetShader((ID3D11HullShader*)tessControlShader->shader, nullptr, 0);
	//if (tessEvalShader != nullptr) context->DSSetShader((ID3D11DomainShader*)tessEvalShader->shader, nullptr, 0);	

	//context->IASetInputLayout(inputLayout);
}

ConstantLocation Program::getConstantLocation(const char* name) {
	char d3dname[101];
	strcpy(d3dname, "_");
	strcat(d3dname, name);
	ConstantLocation location;

	if (vertexShader->constants.find(d3dname) == vertexShader->constants.end()) {
		location.vertexOffset = 0;
		location.vertexSize = 0;
	}
	else {
		ShaderConstant constant = vertexShader->constants[d3dname];
		location.vertexOffset = constant.offset;
		location.vertexSize = constant.size;
	}

	if (fragmentShader->constants.find(d3dname) == fragmentShader->constants.end()) {
		location.fragmentOffset = 0;
		location.fragmentSize = 0;
	}
	else {
		ShaderConstant constant = fragmentShader->constants[d3dname];
		location.fragmentOffset = constant.offset;
		location.fragmentSize = constant.size;
	}

	if (geometryShader == nullptr || geometryShader->constants.find(d3dname) == geometryShader->constants.end()) {
		location.geometryOffset = 0;
		location.geometrySize = 0;
	}
	else {
		ShaderConstant constant = geometryShader->constants[d3dname];
		location.geometryOffset = constant.offset;
		location.geometrySize = constant.size;
	}

	if (tessControlShader == nullptr || tessControlShader->constants.find(d3dname) == tessControlShader->constants.end()) {
		location.tessControlOffset = 0;
		location.tessControlSize = 0;
	}
	else {
		ShaderConstant constant = tessControlShader->constants[d3dname];
		location.tessControlOffset = constant.offset;
		location.tessControlSize = constant.size;
	}

	if (tessEvalShader == nullptr || tessEvalShader->constants.find(d3dname) == tessEvalShader->constants.end()) {
		location.tessEvalOffset = 0;
		location.tessEvalSize = 0;
	}
	else {
		ShaderConstant constant = tessEvalShader->constants[d3dname];
		location.tessEvalOffset = constant.offset;
		location.tessEvalSize = constant.size;
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
}

void Program::link(VertexStructure** structures, int count) {
	D3D12_INPUT_ELEMENT_DESC vertexDesc[10];
	for (int i = 0; i < structures[0]->size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = vertexShader->attributes[structures[0]->elements[i].name];
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (structures[0]->elements[i].data) {
		case Float1VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
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

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.VS.BytecodeLength = vertexShader->length;
	psoDesc.VS.pShaderBytecode = vertexShader->data;
	psoDesc.PS.BytecodeLength = fragmentShader->length;
	psoDesc.PS.pShaderBytecode = fragmentShader->data;
	psoDesc.pRootSignature = rootSignature;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	
	psoDesc.InputLayout.NumElements = structures[0]->size;
	psoDesc.InputLayout.pInputElementDescs = vertexDesc;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
}
