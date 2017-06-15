#include "pch.h"

#include "Direct3D9.h"

#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/WinError.h>

using namespace Kore;

namespace {
	_D3DBLEND convert(Graphics4::BlendingOperation operation) {
		switch (operation) {
		case Graphics4::BlendOne:
			return D3DBLEND_ONE;
		case Graphics4::BlendZero:
			return D3DBLEND_ZERO;
		case Graphics4::SourceAlpha:
			return D3DBLEND_SRCALPHA;
		case Graphics4::DestinationAlpha:
			return D3DBLEND_DESTALPHA;
		case Graphics4::InverseSourceAlpha:
			return D3DBLEND_INVSRCALPHA;
		case Graphics4::InverseDestinationAlpha:
			return D3DBLEND_INVDESTALPHA;
		default:
			//	throw Exception("Unknown blending operation.");
			return D3DBLEND_SRCALPHA;
		}
	}

	_D3DCMPFUNC convert(Graphics4::ZCompareMode mode) {
		switch (mode) {
		default:
		case Graphics4::ZCompareAlways:
			return D3DCMP_ALWAYS;
		case Graphics4::ZCompareNever:
			return D3DCMP_NEVER;
		case Graphics4::ZCompareEqual:
			return D3DCMP_EQUAL;
		case Graphics4::ZCompareNotEqual:
			return D3DCMP_NOTEQUAL;
		case Graphics4::ZCompareLess:
			return D3DCMP_LESS;
		case Graphics4::ZCompareLessEqual:
			return D3DCMP_LESSEQUAL;
		case Graphics4::ZCompareGreater:
			return D3DCMP_GREATER;
		case Graphics4::ZCompareGreaterEqual:
			return D3DCMP_GREATEREQUAL;
		}
	}
}

void Graphics4::PipelineState::compile() {
	int highestIndex = 0;
	for (std::map<std::string, int>::iterator it = vertexShader->attributes.begin(); it != vertexShader->attributes.end(); ++it) {
		if (it->second > highestIndex) {
			highestIndex = it->second;
		}
	}

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

	D3DVERTEXELEMENT9* elements = (D3DVERTEXELEMENT9*)alloca(sizeof(D3DVERTEXELEMENT9) * (all + 1));
	int i = 0;
	for (int stream = 0; inputLayout[stream] != nullptr; ++stream) {
		int stride = 0;
		for (int index = 0; index < inputLayout[stream]->size; ++index) {
			if (inputLayout[stream]->elements[index].data != Float4x4VertexData) {
				elements[i].Stream = stream;
				elements[i].Offset = stride;
			}
			switch (inputLayout[stream]->elements[index].data) {
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
					strcpy(name, inputLayout[stream]->elements[index].name);
					strcat(name, "_");
					size_t length = strlen(name);
					_itoa(i2, &name[length], 10);
					name[length + 1] = 0;
					if (vertexShader->attributes.find(name) == vertexShader->attributes.end()) {
						log(Error, "Could not find attribute %s.", name);
						elements[i].UsageIndex = ++highestIndex;
					}
					else {
						elements[i].UsageIndex = vertexShader->attributes[name];
					}
					stride += 4 * 4;
					++i;
				}
				break;
			}
			if (inputLayout[stream]->elements[index].data != Float4x4VertexData) {
				elements[i].Method = D3DDECLMETHOD_DEFAULT;
				elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
				if (vertexShader->attributes.find(inputLayout[stream]->elements[index].name) == vertexShader->attributes.end()) {
					log(Error, "Could not find attribute %s.", inputLayout[stream]->elements[index].name);
					elements[i].UsageIndex = ++highestIndex;
				}
				else {
					elements[i].UsageIndex = vertexShader->attributes[inputLayout[stream]->elements[index].name];
				}
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

	halfPixelLocation = vertexShader->constants["gl_HalfPixel"].regindex;
}

void PipelineStateImpl::set(Graphics4::PipelineState* pipeline) {
	affirm(device->SetVertexShader((IDirect3DVertexShader9*)pipeline->vertexShader->shader));
	affirm(device->SetPixelShader((IDirect3DPixelShader9*)pipeline->fragmentShader->shader));
	affirm(device->SetVertexDeclaration(vertexDecleration));

	// TODO (DK) System::screenWidth/Height are only main-window dimensions, what about other windows?
	float floats[4];
	floats[0] = 1.0f / System::windowWidth(0);
	floats[1] = 1.0f / System::windowHeight(0);
	floats[2] = floats[0];
	floats[3] = floats[1];
	affirm(device->SetVertexShaderConstantF(halfPixelLocation, floats, 1));

	DWORD flags = 0;
	if (pipeline->colorWriteMaskRed) flags |= D3DCOLORWRITEENABLE_RED;
	if (pipeline->colorWriteMaskGreen) flags |= D3DCOLORWRITEENABLE_GREEN;
	if (pipeline->colorWriteMaskBlue) flags |= D3DCOLORWRITEENABLE_BLUE;
	if (pipeline->colorWriteMaskAlpha) flags |= D3DCOLORWRITEENABLE_ALPHA;

	device->SetRenderState(D3DRS_COLORWRITEENABLE, flags);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, (pipeline->blendSource != Graphics4::BlendOne || pipeline->blendDestination != Graphics4::BlendZero) ? TRUE : FALSE);
	device->SetRenderState(D3DRS_SRCBLEND, convert(pipeline->blendSource));
	device->SetRenderState(D3DRS_DESTBLEND, convert(pipeline->blendDestination));

	switch (pipeline->cullMode) {
	case Graphics4::Clockwise:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		break;
	case Graphics4::CounterClockwise:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		break;
	case Graphics4::NoCulling:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		break;
	}

	device->SetRenderState(D3DRS_ZENABLE, pipeline->depthMode != Graphics4::ZCompareAlways ? TRUE : FALSE);
	device->SetRenderState(D3DRS_ZFUNC, convert(pipeline->depthMode));
	device->SetRenderState(D3DRS_ZWRITEENABLE, pipeline->depthWrite ? TRUE : FALSE);

	/*
	case AlphaTestState:
		device->SetRenderState(D3DRS_ALPHATESTENABLE, on ? TRUE : FALSE);
		device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		break;
	case AlphaReferenceState:
		device->SetRenderState(D3DRS_ALPHAREF, (DWORD)v);
		break;
	*/
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;

	if (fragmentShader->constants.find(name) != fragmentShader->constants.end()) {
		location.reg = fragmentShader->constants[name];
		location.shaderType = 1;
	}
	else if (vertexShader->constants.find(name) != vertexShader->constants.end()) {
		location.reg = vertexShader->constants[name];
		location.shaderType = 0;
	}
	else {
		location.shaderType = -1;
		log(Warning, "Could not find uniform %s.", name);
	}
	return location;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;

	if (fragmentShader->constants.find(name) != fragmentShader->constants.end()) {
		unit.unit = fragmentShader->constants[name].regindex;
	}
	else if (vertexShader->constants.find(name) != vertexShader->constants.end()) {
		unit.unit = vertexShader->constants[name].regindex;
	}
	else {
		unit.unit = -1;
		log(Warning, "Could not find texture %s.", name);
	}
	return unit;
}
