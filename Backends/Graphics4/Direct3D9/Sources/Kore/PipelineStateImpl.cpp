#include "pch.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/log.h>
#include <kinc/system.h>

#include <Kore/SystemMicrosoft.h>

#include "Direct3D9.h"

#include <malloc.h>

namespace {
	_D3DBLEND convert(kinc_g4_blending_operation_t operation) {
		switch (operation) {
		case KINC_G4_BLEND_ONE:
			return D3DBLEND_ONE;
		case KINC_G4_BLEND_ZERO:
			return D3DBLEND_ZERO;
		case KINC_G4_BLEND_SOURCE_ALPHA:
			return D3DBLEND_SRCALPHA;
		case KINC_G4_BLEND_DEST_ALPHA:
			return D3DBLEND_DESTALPHA;
		case KINC_G4_BLEND_INV_SOURCE_ALPHA:
			return D3DBLEND_INVSRCALPHA;
		case KINC_G4_BLEND_INV_DEST_ALPHA:
			return D3DBLEND_INVDESTALPHA;
		default:
			//	throw Exception("Unknown blending operation.");
			return D3DBLEND_SRCALPHA;
		}
	}

	_D3DCMPFUNC convert(kinc_g4_compare_mode_t mode) {
		switch (mode) {
		default:
		case KINC_G4_COMPARE_ALWAYS:
			return D3DCMP_ALWAYS;
		case KINC_G4_COMPARE_NEVER:
			return D3DCMP_NEVER;
		case KINC_G4_COMPARE_EQUAL:
			return D3DCMP_EQUAL;
		case KINC_G4_COMPARE_NOT_EQUAL:
			return D3DCMP_NOTEQUAL;
		case KINC_G4_COMPARE_LESS:
			return D3DCMP_LESS;
		case KINC_G4_COMPARE_LESS_EQUAL:
			return D3DCMP_LESSEQUAL;
		case KINC_G4_COMPARE_GREATER:
			return D3DCMP_GREATER;
		case KINC_G4_COMPARE_GREATER_EQUAL:
			return D3DCMP_GREATEREQUAL;
		}
	}
}

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *state) {}

void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *state) {}

void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *state) {
	int highestIndex = 0;
	for (std::map<std::string, int>::iterator it = vertexShader->attributes.begin(); it != vertexShader->attributes.end(); ++it) {
		if (it->second > highestIndex) {
			highestIndex = it->second;
		}
	}

	int all = 0;
	for (int stream = 0; state->input_layout[stream] != nullptr; ++stream) {
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			if (state->input_layout[stream]->elements[index].data == KINC_G4_VERTEX_DATA_FLOAT4X4) {
				all += 4;
			}
			else {
				all += 1;
			}
		}
	}

	D3DVERTEXELEMENT9 *elements = (D3DVERTEXELEMENT9 *)_alloca(sizeof(D3DVERTEXELEMENT9) * (all + 1));
	int i = 0;
	for (int stream = 0; state->input_layout[stream] != nullptr; ++stream) {
		int stride = 0;
		for (int index = 0; index < state->input_layout[stream]->size; ++index) {
			if (state->input_layout[stream]->elements[index].data != KINC_G4_VERTEX_DATA_FLOAT4X4) {
				elements[i].Stream = stream;
				elements[i].Offset = stride;
			}
			switch (state->input_layout[stream]->elements[index].data) {
			case KINC_G4_VERTEX_DATA_FLOAT1:
				elements[i].Type = D3DDECLTYPE_FLOAT1;
				stride += 4 * 1;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT2:
				elements[i].Type = D3DDECLTYPE_FLOAT2;
				stride += 4 * 2;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT3:
				elements[i].Type = D3DDECLTYPE_FLOAT3;
				stride += 4 * 3;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT4:
				elements[i].Type = D3DDECLTYPE_FLOAT4;
				stride += 4 * 4;
				break;
			case KINC_G4_VERTEX_DATA_COLOR:
				elements[i].Type = D3DDECLTYPE_D3DCOLOR;
				stride += 4;
				break;
			case KINC_G4_VERTEX_DATA_FLOAT4X4:
				for (int i2 = 0; i2 < 4; ++i2) {
					elements[i].Stream = stream;
					elements[i].Offset = stride;
					elements[i].Type = D3DDECLTYPE_FLOAT4;
					elements[i].Method = D3DDECLMETHOD_DEFAULT;
					elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
					char name[101];
					strcpy(name, state->input_layout[stream]->elements[index].name);
					strcat(name, "_");
					size_t length = strlen(name);
					_itoa(i2, &name[length], 10);
					name[length + 1] = 0;
					if (vertexShader->attributes.find(name) == vertexShader->attributes.end()) {
						kinc_log(KINC_LOG_LEVEL_ERROR, "Could not find attribute %s.", name);
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
			if (state->input_layout[stream]->elements[index].data != KINC_G4_VERTEX_DATA_FLOAT4X4) {
				elements[i].Method = D3DDECLMETHOD_DEFAULT;
				elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
				if (vertexShader->attributes.find(state->input_layout[stream]->elements[index].name) == vertexShader->attributes.end()) {
					kinc_log(KINC_LOG_LEVEL_ERROR, "Could not find attribute %s.", state->input_layout[stream]->elements[index].name);
					elements[i].UsageIndex = ++highestIndex;
				}
				else {
					elements[i].UsageIndex = vertexShader->attributes[state->input_layout[stream]->elements[index].name];
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

	state->impl.vertexDecleration = nullptr;
	kinc_microsoft_affirm(device->CreateVertexDeclaration(elements, &state->impl.vertexDecleration));

	state->impl.halfPixelLocation = vertexShader->constants["gl_HalfPixel"].regindex;
}

void kinc_g4_internal_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	kinc_microsoft_affirm(device->SetVertexShader((IDirect3DVertexShader9 *)pipeline->vertex_shader->impl.shader));
	kinc_microsoft_affirm(device->SetPixelShader((IDirect3DPixelShader9 *)pipeline->fragment_shader->impl.shader));
	kinc_microsoft_affirm(device->SetVertexDeclaration(pipeline->impl.vertexDecleration));

	// TODO (DK) System::screenWidth/Height are only main-window dimensions, what about other windows?
	float floats[4];
	floats[0] = 1.0f / kinc_width();
	floats[1] = 1.0f / kinc_height();
	floats[2] = floats[0];
	floats[3] = floats[1];
	kinc_microsoft_affirm(device->SetVertexShaderConstantF(pipeline->impl.halfPixelLocation, floats, 1));

	DWORD flags = 0;
	if (pipeline->color_write_mask_red[0]) flags |= D3DCOLORWRITEENABLE_RED;
	if (pipeline->color_write_mask_green[0]) flags |= D3DCOLORWRITEENABLE_GREEN;
	if (pipeline->color_write_mask_blue[0]) flags |= D3DCOLORWRITEENABLE_BLUE;
	if (pipeline->color_write_mask_alpha[0]) flags |= D3DCOLORWRITEENABLE_ALPHA;

	device->SetRenderState(D3DRS_COLORWRITEENABLE, flags);

	device->SetRenderState(D3DRS_ALPHABLENDENABLE,
	                       (pipeline->blend_source != KINC_G4_BLEND_ONE || pipeline->blend_destination != KINC_G4_BLEND_ZERO) ? TRUE : FALSE);
	device->SetRenderState(D3DRS_SRCBLEND, convert(pipeline->blend_source));
	device->SetRenderState(D3DRS_DESTBLEND, convert(pipeline->blend_destination));

	switch (pipeline->cull_mode) {
	case KINC_G4_CULL_CLOCKWISE:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		break;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		break;
	case KINC_G4_CULL_NOTHING:
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		break;
	}

	device->SetRenderState(D3DRS_ZENABLE, pipeline->depth_mode != KINC_G4_COMPARE_ALWAYS ? TRUE : FALSE);
	device->SetRenderState(D3DRS_ZFUNC, convert(pipeline->depth_mode));
	device->SetRenderState(D3DRS_ZWRITEENABLE, pipeline->depth_write ? TRUE : FALSE);

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

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *state, const char *name) {
	kinc_g4_constant_location_t location;

	if (fragmentShader->constants.find(name) != fragmentShader->constants.end()) {
		location.impl.reg = fragmentShader->constants[name];
		location.impl.shaderType = 1;
	}
	else if (vertexShader->constants.find(name) != vertexShader->constants.end()) {
		location.impl.reg = vertexShader->constants[name];
		location.impl.shaderType = 0;
	}
	else {
		location.impl.shaderType = -1;
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not find uniform %s.", name);
	}
	return location;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *state, const char *name) {
	kinc_g4_texture_unit_t unit;

	if (fragmentShader->constants.find(name) != fragmentShader->constants.end()) {
		unit.impl.unit = fragmentShader->constants[name].regindex;
	}
	else if (vertexShader->constants.find(name) != vertexShader->constants.end()) {
		unit.impl.unit = vertexShader->constants[name].regindex;
	}
	else {
		unit.impl.unit = -1;
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not find texture %s.", name);
	}
	return unit;
}
