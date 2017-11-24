#include "pch.h"

#include "PipelineState5Impl.h"

#include "Direct3D12.h"

#include <Kore/Graphics5/Shader.h>
#include <Kore/Graphics5/PipelineState.h>
#include <Kore/WinError.h>

using namespace Kore;

void PipelineState5Impl::setConstants(ID3D12GraphicsCommandList* commandList, Graphics5::PipelineState* pipeline) {
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

	commandList->SetPipelineState(pipeline->pso);
	commandList->SetGraphicsRootSignature(rootSignature);

	Texture5Impl::setTextures(commandList);
}

PipelineState5Impl::PipelineState5Impl() : vertexShader(nullptr), fragmentShader(nullptr), geometryShader(nullptr), tessEvalShader(nullptr), tessControlShader(nullptr) {}

//void PipelineState5Impl::set(Graphics5::PipelineState* pipeline) {
	//_current = this;
	// context->VSSetShader((ID3D11VertexShader*)vertexShader->shader, nullptr, 0);
	// context->PSSetShader((ID3D11PixelShader*)fragmentShader->shader, nullptr, 0);

	// if (geometryShader != nullptr) context->GSSetShader((ID3D11GeometryShader*)geometryShader->shader, nullptr, 0);
	// if (tessControlShader != nullptr) context->HSSetShader((ID3D11HullShader*)tessControlShader->shader, nullptr, 0);
	// if (tessEvalShader != nullptr) context->DSSetShader((ID3D11DomainShader*)tessEvalShader->shader, nullptr, 0);

	// context->IASetInputLayout(inputLayout);
//}

Graphics5::ConstantLocation Graphics5::PipelineState::getConstantLocation(const char* name) {
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

Graphics5::TextureUnit Graphics5::PipelineState::getTextureUnit(const char* name) {
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

	D3D12_BLEND convert(Graphics5::BlendingOperation op) {
		switch (op) {
		default:
		case Graphics5::BlendOne:
			return D3D12_BLEND_ONE;
		case Graphics5::BlendZero:
			return D3D12_BLEND_ZERO;
		case Graphics5::SourceAlpha:
			return D3D12_BLEND_SRC_ALPHA;
		case Graphics5::DestinationAlpha:
			return D3D12_BLEND_DEST_ALPHA;
		case Graphics5::InverseSourceAlpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case Graphics5::InverseDestinationAlpha:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case Graphics5::SourceColor:
			return D3D12_BLEND_SRC_COLOR;
		case Graphics5::DestinationColor:
			return D3D12_BLEND_DEST_COLOR;
		case Graphics5::InverseSourceColor:
			return D3D12_BLEND_INV_SRC_COLOR;
		case Graphics5::InverseDestinationColor:
			return D3D12_BLEND_INV_DEST_COLOR;
		}
	}
}

void Graphics5::PipelineState::compile() {
	D3D12_INPUT_ELEMENT_DESC vertexDesc[10];
	for (int i = 0; i < inputLayout[0]->size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = vertexShader->attributes[inputLayout[0]->elements[i].name];
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (inputLayout[0]->elements[i].data) {
		case Graphics4::Float1VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case Graphics4::Float2VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case Graphics4::Float3VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case Graphics4::Float4VertexData:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case Graphics4::ColorVertexData:
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
#ifdef KORE_WINDOWS
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
#else
	psoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
#endif
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	psoDesc.InputLayout.NumElements = inputLayout[0]->size;
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
	psoDesc.BlendState.RenderTarget[0].BlendEnable = blendSource != Graphics5::BlendOne || blendDestination != Graphics5::BlendZero || alphaBlendSource != Graphics5::BlendOne || alphaBlendDestination != Graphics5::BlendZero;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = convert(blendSource);
	psoDesc.BlendState.RenderTarget[0].DestBlend = convert(blendDestination);
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = convert(alphaBlendSource);
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = convert(alphaBlendDestination);
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(&pso));
}
