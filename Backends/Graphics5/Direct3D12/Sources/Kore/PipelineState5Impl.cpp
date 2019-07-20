#include "pch.h"

#include "PipelineState5Impl.h"

#include "Direct3D12.h"

#include <Kinc/Graphics5/Pipeline.h>
#include <Kinc/Graphics5/Shader.h>

#include <Kore/SystemMicrosoft.h>

void kinc_g5_internal_setConstants(ID3D12GraphicsCommandList *commandList, kinc_g5_pipeline_t *pipeline) {
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

	commandList->SetPipelineState(pipeline->impl.pso);
	commandList->SetGraphicsRootSignature(rootSignature);

	kinc_g5_internal_set_textures(commandList);
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	pipe->vertexShader = nullptr;
	pipe->fragmentShader = nullptr;
	pipe->geometryShader = nullptr;
	pipe->tessellationEvaluationShader = nullptr;
	pipe->tessellationControlShader = nullptr;
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipe) {
	if (pipe->impl.pso != nullptr) {
		pipe->impl.pso->Release();
		pipe->impl.pso = nullptr;
	}
}

// void PipelineState5Impl::set(Graphics5::PipelineState* pipeline) {
//_current = this;
// context->VSSetShader((ID3D11VertexShader*)vertexShader->shader, nullptr, 0);
// context->PSSetShader((ID3D11PixelShader*)fragmentShader->shader, nullptr, 0);

// if (geometryShader != nullptr) context->GSSetShader((ID3D11GeometryShader*)geometryShader->shader, nullptr, 0);
// if (tessControlShader != nullptr) context->HSSetShader((ID3D11HullShader*)tessControlShader->shader, nullptr, 0);
// if (tessEvalShader != nullptr) context->DSSetShader((ID3D11DomainShader*)tessEvalShader->shader, nullptr, 0);

// context->IASetInputLayout(inputLayout);
//}

#define MAX_SHADER_THING 32

static ShaderConstant findConstant(kinc_g5_shader_t *shader, const char *name) {
	if (shader != NULL) {
		for (int i = 0; i < MAX_SHADER_THING; ++i) {
			if (strcmp(shader->impl.constants[i].name, name) == 0) {
				return shader->impl.constants[i];
			}
		}
	}

	ShaderConstant constant;
	constant.name[0] = 0;
	constant.offset = -1;
	constant.size = 0;
	return constant;
}

static ShaderTexture findTexture(kinc_g5_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.textures[i].name, name) == 0) {
			return shader->impl.textures[i];
		}
	}

	ShaderTexture texture;
	texture.name[0] = 0;
	texture.texture = -1;
	return texture;
}

static ShaderAttribute findAttribute(kinc_g5_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.attributes[i].name, name) == 0) {
			return shader->impl.attributes[i];
		}
	}

	ShaderAttribute attribute;
	attribute.name[0] = 0;
	attribute.attribute = -1;
	return attribute;
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline *pipe, const char *name) {
	kinc_g5_constant_location_t location;

	{
		ShaderConstant constant = findConstant(pipe->vertexShader, name);
		location.impl.vertexOffset = constant.offset;
		location.impl.vertexSize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->fragmentShader, name);
		location.impl.fragmentOffset = constant.offset;
		location.impl.fragmentSize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->geometryShader, name);
		location.impl.geometryOffset = constant.offset;
		location.impl.geometrySize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->tessellationControlShader, name);
		location.impl.tessControlOffset = constant.offset;
		location.impl.tessControlSize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->tessellationEvaluationShader, name);
		location.impl.tessEvalOffset = constant.offset;
		location.impl.tessEvalSize = constant.size;
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	ShaderTexture vertexTexture = findTexture(pipe->vertexShader, name);
	if (vertexTexture.texture != -1) {
		unit.impl.unit = vertexTexture.texture;
	}
	else {
		ShaderTexture fragmentTexture = findTexture(pipe->fragmentShader, name);
		unit.impl.unit = fragmentTexture.texture;
	}
	return unit;
}

namespace {
	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	D3D12_BLEND convert(kinc_g5_blending_operation_t op) {
		switch (op) {
		default:
		case KINC_G5_BLEND_MODE_ONE:
			return D3D12_BLEND_ONE;
		case KINC_G5_BLEND_MODE_ZERO:
			return D3D12_BLEND_ZERO;
		case KINC_G5_BLEND_MODE_SOURCE_ALPHA:
			return D3D12_BLEND_SRC_ALPHA;
		case KINC_G5_BLEND_MODE_DEST_ALPHA:
			return D3D12_BLEND_DEST_ALPHA;
		case KINC_G5_BLEND_MODE_INV_SOURCE_ALPHA:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case KINC_G5_BLEND_MODE_INV_DEST_ALPHA:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case KINC_G5_BLEND_MODE_SOURCE_COLOR:
			return D3D12_BLEND_SRC_COLOR;
		case KINC_G5_BLEND_MODE_DEST_COLOR:
			return D3D12_BLEND_DEST_COLOR;
		case KINC_G5_BLEND_MODE_INV_SOURCE_COLOR:
			return D3D12_BLEND_INV_SRC_COLOR;
		case KINC_G5_BLEND_MODE_INV_DEST_COLOR:
			return D3D12_BLEND_INV_DEST_COLOR;
		}
	}
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
	D3D12_INPUT_ELEMENT_DESC vertexDesc[10];
	for (int i = 0; i < pipe->inputLayout[0]->size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = findAttribute(pipe->vertexShader, pipe->inputLayout[0]->elements[i].name).attribute;
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (pipe->inputLayout[0]->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case KINC_G4_VERTEX_DATA_COLOR:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		}
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.VS.BytecodeLength = pipe->vertexShader->impl.length;
	psoDesc.VS.pShaderBytecode = pipe->vertexShader->impl.data;
	psoDesc.PS.BytecodeLength = pipe->fragmentShader->impl.length;
	psoDesc.PS.pShaderBytecode = pipe->fragmentShader->impl.data;
	psoDesc.pRootSignature = rootSignature;
	psoDesc.NumRenderTargets = 1;
#ifdef KORE_WINDOWS
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
#else
	psoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
#endif
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	psoDesc.InputLayout.NumElements = pipe->inputLayout[0]->size;
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
	psoDesc.BlendState.RenderTarget[0].BlendEnable = pipe->blendSource != KINC_G5_BLEND_MODE_ONE || pipe->blendDestination != KINC_G5_BLEND_MODE_ZERO ||
	                                                 pipe->alphaBlendSource != KINC_G5_BLEND_MODE_ONE || pipe->alphaBlendDestination != KINC_G5_BLEND_MODE_ZERO;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = convert(pipe->blendSource);
	psoDesc.BlendState.RenderTarget[0].DestBlend = convert(pipe->blendDestination);
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = convert(pipe->alphaBlendSource);
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = convert(pipe->alphaBlendDestination);
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = pipe->depthMode != KINC_G5_COMPARE_MODE_ALWAYS;
	psoDesc.DepthStencilState.DepthWriteMask = pipe->depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(&pipe->impl.pso));
}
