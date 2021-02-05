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

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *state) {
	memset(state, 0, sizeof(kinc_g4_pipeline));
	kinc_g4_internal_pipeline_set_defaults(state);
}

void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *state) {}

static int find_attribute(struct ShaderAttribute *attributes, const char *name) {
	for (int i = 0; i < KINC_INTERNAL_MAX_ATTRIBUTES; ++i) {
		if (attributes[i].name == 0) {
			return -1;
		}
		if (strcmp(attributes[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

static int find_constant(struct ShaderRegister *constants, const char *name) {
	for (int i = 0; i < KINC_INTERNAL_MAX_CONSTANTS; ++i) {
		if (constants[i].name == 0) {
			return -1;
		}
		if (strcmp(constants[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *state) {
	int highestIndex = 0;
	for (int i = 0;; ++i) {
		if (state->vertex_shader->impl.attributes[i].name[0] == 0) {
			break;
		}
		int index = state->vertex_shader->impl.attributes[i].index;
		if (index > highestIndex) {
			highestIndex = index;
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
					int attribute_index = find_attribute(state->vertex_shader->impl.attributes, name);
					if (attribute_index < 0) {
						kinc_log(KINC_LOG_LEVEL_ERROR, "Could not find attribute %s.", name);
						elements[i].UsageIndex = ++highestIndex;
					}
					else {
						elements[i].UsageIndex = state->vertex_shader->impl.attributes[attribute_index].index;
					}
					stride += 4 * 4;
					++i;
				}
				break;
			}
			if (state->input_layout[stream]->elements[index].data != KINC_G4_VERTEX_DATA_FLOAT4X4) {
				elements[i].Method = D3DDECLMETHOD_DEFAULT;
				elements[i].Usage = D3DDECLUSAGE_TEXCOORD;
				int attribute_index = find_attribute(state->vertex_shader->impl.attributes, state->input_layout[stream]->elements[index].name);
				if (attribute_index < 0) {
					kinc_log(KINC_LOG_LEVEL_ERROR, "Could not find attribute %s.", state->input_layout[stream]->elements[index].name);
					elements[i].UsageIndex = ++highestIndex;
				}
				else {
					elements[i].UsageIndex = state->vertex_shader->impl.attributes[attribute_index].index;
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

	int constant_index = find_constant(state->vertex_shader->impl.constants, "gl_HalfPixel");
	state->impl.halfPixelLocation = state->vertex_shader->impl.constants[constant_index].regindex;
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
	if (pipeline->color_write_mask_red[0]) {
		flags |= D3DCOLORWRITEENABLE_RED;
	}
	if (pipeline->color_write_mask_green[0]) {
		flags |= D3DCOLORWRITEENABLE_GREEN;
	}
	if (pipeline->color_write_mask_blue[0]) {
		flags |= D3DCOLORWRITEENABLE_BLUE;
	}
	if (pipeline->color_write_mask_alpha[0]) {
		flags |= D3DCOLORWRITEENABLE_ALPHA;
	}

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

	int fragment_index = find_constant(state->fragment_shader->impl.constants, name);
	int vertex_index = find_constant(state->vertex_shader->impl.constants, name);

	if (fragment_index >= 0) {
		location.impl.reg = state->fragment_shader->impl.constants[fragment_index];
		location.impl.shaderType = 1;
	}
	else if (vertex_index >= 0) {
		location.impl.reg = state->vertex_shader->impl.constants[vertex_index];
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

	int fragment_index = find_constant(state->fragment_shader->impl.constants, name);
	int vertex_index = find_constant(state->vertex_shader->impl.constants, name);

	if (fragment_index >= 0) {
		unit.impl.unit = state->fragment_shader->impl.constants[fragment_index].regindex;
	}
	else if (vertex_index >= 0) {
		unit.impl.unit = state->vertex_shader->impl.constants[vertex_index].regindex;
	}
	else {
		unit.impl.unit = -1;
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not find texture %s.", name);
	}
	return unit;
}
