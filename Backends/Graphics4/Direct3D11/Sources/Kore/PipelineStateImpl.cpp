#include "pch.h"

#include "Direct3D11.h"

#include <Kinc/Graphics4/PipelineState.h>
#include <Kinc/Graphics4/Shader.h>

#include <Kore/SystemMicrosoft.h>
#include <Kore/Log.h>

#include <assert.h>
#include <malloc.h>

Kinc_G4_PipelineState *currentPipeline = NULL;

static D3D11_CULL_MODE convert_cull_mode(Kinc_G4_CullMode cullMode) {
	switch (cullMode) {
	case KINC_G4_CULL_CLOCKWISE:
		return D3D11_CULL_FRONT;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		return D3D11_CULL_BACK;
	case KINC_G4_CULL_NOTHING:
		return D3D11_CULL_NONE;
	default:
		assert(false);
	}
}

static D3D11_BLEND convert_blend_operation(Kinc_G4_BlendingOperation operation) {
	switch (operation) {
	case KINC_G4_BLEND_ONE:
		return D3D11_BLEND_ONE;
	case KINC_G4_BLEND_ZERO:
		return D3D11_BLEND_ZERO;
	case KINC_G4_BLEND_SOURCE_ALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case KINC_G4_BLEND_DEST_ALPHA:
		return D3D11_BLEND_DEST_ALPHA;
	case KINC_G4_BLEND_INV_SOURCE_ALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case KINC_G4_BLEND_INV_DEST_ALPHA:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case KINC_G4_BLEND_SOURCE_COLOR:
		return D3D11_BLEND_SRC_COLOR;
	case KINC_G4_BLEND_DEST_COLOR:
		return D3D11_BLEND_DEST_COLOR;
	case KINC_G4_BLEND_INV_SOURCE_COLOR:
		return D3D11_BLEND_INV_SRC_COLOR;
	case KINC_G4_BLEND_INV_DEST_COLOR:
		return D3D11_BLEND_INV_DEST_COLOR;
	default:
		//	throw Exception("Unknown blending operation.");
		return D3D11_BLEND_SRC_ALPHA;
	}
}

static D3D11_COMPARISON_FUNC get_comparison(Kinc_G4_CompareMode compare) {
	switch (compare) {
	default:
	case KINC_G4_COMPARE_ALWAYS:
		return D3D11_COMPARISON_ALWAYS;
	case KINC_G4_COMPARE_NEVER:
		return D3D11_COMPARISON_NEVER;
	case KINC_G4_COMPARE_EQUAL:
		return D3D11_COMPARISON_EQUAL;
	case KINC_G4_COMPARE_NOT_EQUAL:
		return D3D11_COMPARISON_NOT_EQUAL;
	case KINC_G4_COMPARE_LESS:
		return D3D11_COMPARISON_LESS;
	case KINC_G4_COMPARE_LESS_EQUAL:
		return D3D11_COMPARISON_LESS_EQUAL;
	case KINC_G4_COMPARE_GREATER:
		return D3D11_COMPARISON_GREATER;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		return D3D11_COMPARISON_GREATER_EQUAL;
	}
}

static D3D11_STENCIL_OP get_stencil_action(Kinc_G4_StencilAction action) {
	switch (action) {
	default:
	case KINC_G4_STENCIL_KEEP:
		return D3D11_STENCIL_OP_KEEP;
	case KINC_G4_STENCIL_ZERO:
		return D3D11_STENCIL_OP_ZERO;
	case KINC_G4_STENCIL_REPLACE:
		return D3D11_STENCIL_OP_REPLACE;
	case KINC_G4_STENCIL_INCREMENT:
		return D3D11_STENCIL_OP_INCR;
	case KINC_G4_STENCIL_INCREMENT_WRAP:
		return D3D11_STENCIL_OP_INCR_SAT;
	case KINC_G4_STENCIL_DECREMENT:
		return D3D11_STENCIL_OP_DECR;
	case KINC_G4_STENCIL_DECREMENT_WRAP:
		return D3D11_STENCIL_OP_DECR_SAT;
	case KINC_G4_STENCIL_INVERT:
		return D3D11_STENCIL_OP_INVERT;
	}
}

void Kinc_Internal_SetConstants(void) {
	if (currentPipeline->vertexShader->impl.constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->impl.vertexConstantBuffer, 0, nullptr, vertexConstants, 0, 0);
		context->VSSetConstantBuffers(0, 1, &currentPipeline->impl.vertexConstantBuffer);
	}
	if (currentPipeline->fragmentShader->impl.constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->impl.fragmentConstantBuffer, 0, nullptr, fragmentConstants, 0, 0);
		context->PSSetConstantBuffers(0, 1, &currentPipeline->impl.fragmentConstantBuffer);
	}
	if (currentPipeline->geometryShader != nullptr && currentPipeline->geometryShader->impl.constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->impl.geometryConstantBuffer, 0, nullptr, geometryConstants, 0, 0);
		context->GSSetConstantBuffers(0, 1, &currentPipeline->impl.geometryConstantBuffer);
	}
	if (currentPipeline->tessellationControlShader != nullptr && currentPipeline->tessellationControlShader->impl.constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->impl.tessControlConstantBuffer, 0, nullptr, tessControlConstants, 0, 0);
		context->HSSetConstantBuffers(0, 1, &currentPipeline->impl.tessControlConstantBuffer);
	}
	if (currentPipeline->tessellationEvaluationShader != nullptr && currentPipeline->tessellationEvaluationShader->impl.constantsSize > 0) {
		context->UpdateSubresource(currentPipeline->impl.tessEvalConstantBuffer, 0, nullptr, tessEvalConstants, 0, 0);
		context->DSSetConstantBuffers(0, 1, &currentPipeline->impl.tessEvalConstantBuffer);
	}
}

void Kinc_G4_PipelineState_Create(Kinc_G4_PipelineState *state) {
	state->impl.d3d11inputLayout = nullptr;
	state->impl.fragmentConstantBuffer = nullptr;
	state->impl.vertexConstantBuffer = nullptr;
	state->impl.geometryConstantBuffer = nullptr;
	state->impl.tessEvalConstantBuffer = nullptr;
	state->impl.tessControlConstantBuffer = nullptr;
	state->impl.depthStencilState = nullptr;
	state->impl.rasterizerState = nullptr;
	state->impl.rasterizerStateScissor = nullptr;
	state->impl.blendState = nullptr;
}

void Kinc_G4_PipelineState_Destroy(Kinc_G4_PipelineState *state) {
	if (state->impl.d3d11inputLayout != nullptr) {
		state->impl.d3d11inputLayout->Release();
		state->impl.d3d11inputLayout = NULL;
	}
	if (state->impl.fragmentConstantBuffer != nullptr) {
		state->impl.fragmentConstantBuffer->Release();
		state->impl.fragmentConstantBuffer = NULL;
	}
	if (state->impl.vertexConstantBuffer != nullptr) {
		state->impl.vertexConstantBuffer->Release();
		state->impl.vertexConstantBuffer = NULL;
	}
	if (state->impl.geometryConstantBuffer != nullptr) {
		state->impl.geometryConstantBuffer->Release();
		state->impl.geometryConstantBuffer = NULL;
	}
	if (state->impl.tessEvalConstantBuffer != nullptr) {
		state->impl.tessEvalConstantBuffer->Release();
		state->impl.tessEvalConstantBuffer = NULL;
	}
	if (state->impl.tessControlConstantBuffer != nullptr) {
		state->impl.tessControlConstantBuffer->Release();
		state->impl.tessControlConstantBuffer = NULL;
	}
	if (state->impl.depthStencilState != nullptr) {
		state->impl.depthStencilState->Release();
		state->impl.depthStencilState = NULL;
	}
	if (state->impl.rasterizerState != nullptr) {
		state->impl.rasterizerState->Release();
		state->impl.rasterizerState = NULL;
	}
	if (state->impl.rasterizerStateScissor != nullptr) {
		state->impl.rasterizerStateScissor->Release();
		state->impl.rasterizerStateScissor = NULL;
	}
	if (state->impl.blendState != nullptr) {
		state->impl.blendState->Release();
		state->impl.blendState = NULL;
	}
}

static void setRasterizerState(Kinc_G4_PipelineState *pipeline, bool scissoring) {
	if (scissoring && pipeline->impl.rasterizerStateScissor != nullptr)
		context->RSSetState(pipeline->impl.rasterizerStateScissor);
	else if (pipeline->impl.rasterizerState != nullptr)
		context->RSSetState(pipeline->impl.rasterizerState);
}

void Kinc_Internal_SetPipeline(Kinc_G4_PipelineState *pipeline, bool scissoring) {
	currentPipeline = pipeline;

	context->OMSetDepthStencilState(pipeline->impl.depthStencilState, pipeline->stencilReferenceValue);
	float blendFactor[] = {0, 0, 0, 0};
	UINT sampleMask = 0xffffffff;
	context->OMSetBlendState(pipeline->impl.blendState, blendFactor, sampleMask);
	setRasterizerState(pipeline, scissoring);

	context->VSSetShader((ID3D11VertexShader*)pipeline->vertexShader->impl.shader, nullptr, 0);
	context->PSSetShader((ID3D11PixelShader*)pipeline->fragmentShader->impl.shader, nullptr, 0);
	
	context->GSSetShader(pipeline->geometryShader != nullptr ? (ID3D11GeometryShader*) pipeline->geometryShader->impl.shader : nullptr, nullptr, 0);
	context->HSSetShader(pipeline->tessellationControlShader != nullptr ? (ID3D11HullShader*) pipeline->tessellationControlShader->impl.shader : nullptr, nullptr, 0);
	context->DSSetShader(pipeline->tessellationEvaluationShader != nullptr ? (ID3D11DomainShader*) pipeline->tessellationEvaluationShader->impl.shader : nullptr, nullptr, 0);		

	context->IASetInputLayout(pipeline->impl.d3d11inputLayout);
}

Kinc_G4_ConstantLocation Kinc_G4_PipelineState_GetConstantLocation(Kinc_G4_PipelineState *state, const char *name) {
	Kinc_G4_ConstantLocation location;

	if (state->vertexShader->impl.constants.find(name) == state->vertexShader->impl.constants.end()) {
		location.impl.vertexOffset = 0;
		location.impl.vertexSize = 0;
		location.impl.vertexColumns = 0;
		location.impl.vertexRows = 0;
	}
	else {
		Kinc_Internal_ShaderConstant constant = vertexShader->constants[name];
		location.impl.vertexOffset = constant.offset;
		location.impl.vertexSize = constant.size;
		location.impl.vertexColumns = constant.columns;
		location.impl.vertexRows = constant.rows;
	}

	if (fragmentShader->constants.find(name) == fragmentShader->constants.end()) {
		location.impl.fragmentOffset = 0;
		location.impl.fragmentSize = 0;
		location.impl.fragmentColumns = 0;
		location.impl.fragmentRows = 0;
	}
	else {
		Kinc_Internal_ShaderConstant constant = fragmentShader->constants[name];
		location.impl.fragmentOffset = constant.offset;
		location.impl.fragmentSize = constant.size;
		location.impl.fragmentColumns = constant.columns;
		location.impl.fragmentRows = constant.rows;
	}

	if (geometryShader == nullptr || geometryShader->constants.find(name) == geometryShader->constants.end()) {
		location.impl.geometryOffset = 0;
		location.impl.geometrySize = 0;
		location.impl.geometryColumns = 0;
		location.impl.geometryRows = 0;
	}
	else {
		Kinc_Internal_ShaderConstant constant = geometryShader->constants[name];
		location.impl.geometryOffset = constant.offset;
		location.impl.geometrySize = constant.size;
		location.impl.geometryColumns = constant.columns;
		location.impl.geometryRows = constant.rows;
	}

	if (tessellationControlShader == nullptr || tessellationControlShader->constants.find(name) == tessellationControlShader->constants.end()) {
		location.impl.tessControlOffset = 0;
		location.impl.tessControlSize = 0;
		location.impl.tessControlColumns = 0;
		location.impl.tessControlRows = 0;
	}
	else {
		Kinc_Internal_ShaderConstant constant = tessellationControlShader->constants[name];
		location.impl.tessControlOffset = constant.offset;
		location.impl.tessControlSize = constant.size;
		location.impl.tessControlColumns = constant.columns;
		location.impl.tessControlRows = constant.rows;
	}

	if (tessellationEvaluationShader == nullptr || tessellationEvaluationShader->constants.find(name) == tessellationEvaluationShader->constants.end()) {
		location.impl.tessEvalOffset = 0;
		location.impl.tessEvalSize = 0;
		location.impl.tessEvalColumns = 0;
		location.impl.tessEvalRows = 0;
	}
	else {
		Kinc_Internal_ShaderConstant constant = tessellationEvaluationShader->constants[name];
		location.impl.tessEvalOffset = constant.offset;
		location.impl.tessEvalSize = constant.size;
		location.impl.tessEvalColumns = constant.columns;
		location.impl.tessEvalRows = constant.rows;
	}

	if (location.vertexSize == 0 && location.fragmentSize == 0 && location.geometrySize == 0 && location.tessControlSize && location.tessEvalSize == 0) {
		log(Warning, "Uniform %s not found.", name);
	}

	return location;
}

Kinc_G4_TextureUnit Kinc_G4_PipelineState_GetTextureUnit(Kinc_G4_PipelineState *state, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63) len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') { // Check for array - mySampler[2]
		unitOffset = int(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0; // Strip array from name
	}
	
	Kinc_G4_TextureUnit unit;
	if (vertexShader->textures.find(unitName) == vertexShader->textures.end()) {
		if (fragmentShader->textures.find(unitName) == fragmentShader->textures.end()) {
			unit.unit = -1;
#ifndef NDEBUG
			static int notFoundCount = 0;
			if (notFoundCount < 10) {
				log(Warning, "Sampler %s not found.", unitName);
				++notFoundCount;
			}
			else if (notFoundCount == 10) {
				log(Warning, "Giving up on sampler not found messages.", unitName);
				++notFoundCount;
			}
#endif
		}
		else {
			unit.unit = fragmentShader->textures[unitName] + unitOffset;
			unit.vertex = false;
		}
	}
	else {
		unit.unit = vertexShader->textures[unitName] + unitOffset;
		unit.vertex = true;
	}
	return unit;
}

namespace {
	char stringCache[1024];
	int stringCacheIndex = 0;

	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	void setVertexDesc(D3D11_INPUT_ELEMENT_DESC& vertexDesc, int attributeIndex, int index, int stream, bool instanced, int subindex = -1) {
		if (subindex < 0) {
			vertexDesc.SemanticName = "TEXCOORD";
			vertexDesc.SemanticIndex = attributeIndex;
		}
		else {
			// SPIRV_CROSS uses TEXCOORD_0_0,... for split up matrices
			int stringStart = stringCacheIndex;
			strcpy(&stringCache[stringCacheIndex], "TEXCOORD");
			stringCacheIndex += (int)strlen("TEXCOORD");
			_itoa(attributeIndex, &stringCache[stringCacheIndex], 10);
			stringCacheIndex += (int)strlen(&stringCache[stringCacheIndex]);
			strcpy(&stringCache[stringCacheIndex], "_");
			stringCacheIndex += 2;
			vertexDesc.SemanticName = &stringCache[stringStart];
			vertexDesc.SemanticIndex = subindex;
		}
		vertexDesc.InputSlot = stream;
		vertexDesc.AlignedByteOffset = (index == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc.InputSlotClass = instanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
		vertexDesc.InstanceDataStepRate = instanced ? 1 : 0;
	}
}

namespace {
	const int usedCount = 32;

	int getAttributeLocation(std::map<std::string, int>& attributes, const char* name, bool* used) {
		if (attributes.find(name) != attributes.end()) {
			return attributes[name];
		}
		else {
			for (int i = 0; i < usedCount; ++i) {
				if (!used[i]) {
					used[i] = true;
					return i;
				}
			}
		}
		return 0;
	}

	void createRenderTargetBlendDesc(Graphics4::PipelineState* pipe, D3D11_RENDER_TARGET_BLEND_DESC* rtbd, int targetNum) {
		rtbd->BlendEnable = pipe->blendSource != Graphics4::BlendOne ||
		                    pipe->blendDestination != Graphics4::BlendZero ||
		                    pipe->alphaBlendSource != Graphics4::BlendOne ||
		                    pipe->alphaBlendDestination != Graphics4::BlendZero;
		rtbd->SrcBlend = convert(pipe->blendSource);
		rtbd->DestBlend = convert(pipe->blendDestination);
		rtbd->BlendOp = D3D11_BLEND_OP_ADD;
		rtbd->SrcBlendAlpha = convert(pipe->alphaBlendSource);
		rtbd->DestBlendAlpha = convert(pipe->alphaBlendDestination);
		rtbd->BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd->RenderTargetWriteMask = (((pipe->colorWriteMaskRed[targetNum] ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
		                                (pipe->colorWriteMaskGreen[targetNum] ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0)) |
		                                (pipe->colorWriteMaskBlue[targetNum] ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0)) |
		                                (pipe->colorWriteMaskAlpha[targetNum] ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
	}
}

void Kinc_G4_PipelineState_compile(Kinc_G4_PipelineState *state) {
	if (vertexShader->constantsSize > 0)
		Kinc_Microsoft_Affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(vertexShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                                       &vertexConstantBuffer));
	if (fragmentShader->constantsSize > 0)
		Kinc_Microsoft_Affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(fragmentShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                                       &fragmentConstantBuffer));
	if (geometryShader != nullptr && geometryShader->constantsSize > 0)
		Kinc_Microsoft_Affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(geometryShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
		                                       &geometryConstantBuffer));
	if (tessellationControlShader != nullptr && tessellationControlShader->constantsSize > 0)
		Kinc_Microsoft_Affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(tessellationControlShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER),
		                                       nullptr, &tessControlConstantBuffer));
	if (tessellationEvaluationShader != nullptr && tessellationEvaluationShader->constantsSize > 0)
		Kinc_Microsoft_Affirm(device->CreateBuffer(
		    &CD3D11_BUFFER_DESC(getMultipleOf16(tessellationEvaluationShader->constantsSize), D3D11_BIND_CONSTANT_BUFFER),
		                                       nullptr, &tessEvalConstantBuffer));

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

	bool used[usedCount];
	for (int i = 0; i < usedCount; ++i) used[i] = false;
	for (auto attribute : vertexShader->attributes) {
		used[attribute.second] = true;
	}
	stringCacheIndex = 0;
	D3D11_INPUT_ELEMENT_DESC* vertexDesc = (D3D11_INPUT_ELEMENT_DESC*)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC) * all);
	int i = 0;
	for (int stream = 0; inputLayout[stream] != nullptr; ++stream) {
		for (int index = 0; index < inputLayout[stream]->size; ++index) {
			switch (inputLayout[stream]->elements[index].data) {
			case Float1VertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
				++i;
				break;
			case Float2VertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
				++i;
				break;
			case Float3VertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				++i;
				break;
			case Float4VertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				++i;
				break;
			case Short2NormVertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
				++i;
				break;
			case Short4NormVertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
				++i;
				break;
			case ColorVertexData:
				setVertexDesc(vertexDesc[i], getAttributeLocation(vertexShader->attributes, inputLayout[stream]->elements[index].name, used), index, stream,
				              inputLayout[stream]->instanced);
				vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
				++i;
				break;
			case Float4x4VertexData:
				char name[101];
				strcpy(name, inputLayout[stream]->elements[index].name);
				strcat(name, "_");
				size_t length = strlen(name);
				_itoa(0, &name[length], 10);
				name[length + 1] = 0;
				int attributeLocation = getAttributeLocation(vertexShader->attributes, name, used);

				for (int i2 = 0; i2 < 4; ++i2) {
					setVertexDesc(vertexDesc[i], attributeLocation, index + i2, stream,
					              inputLayout[stream]->instanced, i2);
					vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					++i;
				}
				break;
			}
		}
	}

	Kinc_Microsoft_Affirm(device->CreateInputLayout(vertexDesc, all, vertexShader->data, vertexShader->length, &d3d11inputLayout));

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = depthMode != ZCompareAlways;
		desc.DepthWriteMask = depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = getComparison(depthMode);

		desc.StencilEnable = stencilMode != ZCompareAlways;
		desc.StencilReadMask = stencilReadMask;
		desc.StencilWriteMask = stencilWriteMask;
		desc.FrontFace.StencilFunc = desc.BackFace.StencilFunc = getComparison(stencilMode);
		desc.FrontFace.StencilDepthFailOp = desc.BackFace.StencilDepthFailOp = getStencilAction(stencilDepthFail);
		desc.FrontFace.StencilPassOp = desc.BackFace.StencilPassOp = getStencilAction(stencilBothPass);
		desc.FrontFace.StencilFailOp = desc.BackFace.StencilFailOp = getStencilAction(stencilFail);

		device->CreateDepthStencilState(&desc, &depthStencilState);
	}

	{
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.CullMode = convert(cullMode);
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.ScissorEnable = FALSE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;

		device->CreateRasterizerState(&rasterDesc, &rasterizerState);
		rasterDesc.ScissorEnable = TRUE;
		device->CreateRasterizerState(&rasterDesc, &rasterizerStateScissor);

		// We need d3d11_3 for conservative raster
		// D3D11_RASTERIZER_DESC2 rasterDesc;
		// rasterDesc.ConservativeRaster = conservativeRasterization ? D3D11_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		// device->CreateRasterizerState2(&rasterDesc, &rasterizerState);
		// rasterDesc.ScissorEnable = TRUE;
		// device->CreateRasterizerState2(&rasterDesc, &rasterizerStateScissor);
	}

	{
		bool independentBlend = false;
		for (int i = 1; i < 8; ++i) {
			if (colorWriteMaskRed[0] != colorWriteMaskRed[i] ||
			    colorWriteMaskGreen[0] != colorWriteMaskGreen[i] ||
			    colorWriteMaskBlue[0] != colorWriteMaskBlue[i] ||
			    colorWriteMaskAlpha[0] != colorWriteMaskAlpha[i]) {
				independentBlend = true;
				break;
			}
		}

		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = independentBlend;

		D3D11_RENDER_TARGET_BLEND_DESC rtbd[8];
		ZeroMemory(&rtbd, sizeof(rtbd));
		createRenderTargetBlendDesc(this, &rtbd[0], 0);
		blendDesc.RenderTarget[0] = rtbd[0];
		if (independentBlend) {
			for (int i = 1; i < 8; ++i) {
				createRenderTargetBlendDesc(this, &rtbd[i], i);
				blendDesc.RenderTarget[i] = rtbd[i];
			}
		}

		device->CreateBlendState(&blendDesc, &blendState);
	}
}
