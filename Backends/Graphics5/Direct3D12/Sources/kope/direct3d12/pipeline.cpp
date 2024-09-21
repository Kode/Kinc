#include "pipeline_functions.h"
#include "pipeline_structs.h"

#include <kinc/log.h>

#include <kinc/backend/SystemMicrosoft.h>

#include <kong_ray.h>

static D3D12_BLEND convert_blend_factor(kope_d3d12_blend_factor factor) {
	switch (factor) {
	case KOPE_D3D12_BLEND_FACTOR_ZERO:
		return D3D12_BLEND_ZERO;
	case KOPE_D3D12_BLEND_FACTOR_ONE:
		return D3D12_BLEND_ONE;
	case KOPE_D3D12_BLEND_FACTOR_SRC:
		return D3D12_BLEND_SRC_COLOR;
	case KOPE_D3D12_BLEND_FACTOR_ONE_MINUS_SRC:
		return D3D12_BLEND_INV_SRC_COLOR;
	case KOPE_D3D12_BLEND_FACTOR_SRC_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case KOPE_D3D12_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case KOPE_D3D12_BLEND_FACTOR_DST:
		return D3D12_BLEND_DEST_COLOR;
	case KOPE_D3D12_BLEND_FACTOR_ONE_MINUS_DST:
		return D3D12_BLEND_INV_DEST_COLOR;
	case KOPE_D3D12_BLEND_FACTOR_DST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case KOPE_D3D12_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case KOPE_D3D12_BLEND_FACTOR_SRC_ALPHA_SATURATED:
		return D3D12_BLEND_SRC_ALPHA_SAT;
	case KOPE_D3D12_BLEND_FACTOR_CONSTANT:
		return D3D12_BLEND_BLEND_FACTOR;
	case KOPE_D3D12_BLEND_FACTOR_ONE_MINUS_CONSTANT:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	default:
		assert(false);
		return D3D12_BLEND_ONE;
	}
}

static D3D12_BLEND_OP convert_blend_operation(kope_d3d12_blend_operation op) {
	switch (op) {
	case KOPE_D3D12_BLEND_OPERATION_ADD:
		return D3D12_BLEND_OP_ADD;
	case KOPE_D3D12_BLEND_OPERATION_SUBTRACT:
		return D3D12_BLEND_OP_SUBTRACT;
	case KOPE_D3D12_BLEND_OPERATION_REVERSE_SUBTRACT:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case KOPE_D3D12_BLEND_OPERATION_MIN:
		return D3D12_BLEND_OP_MIN;
	case KOPE_D3D12_BLEND_OPERATION_MAX:
		return D3D12_BLEND_OP_MAX;
	default:
		assert(false);
		return D3D12_BLEND_OP_ADD;
	}
}

static D3D12_CULL_MODE convert_cull_mode(kope_d3d12_cull_mode mode) {
	switch (mode) {
	case KOPE_D3D12_CULL_MODE_NONE:
		return D3D12_CULL_MODE_NONE;
	case KOPE_D3D12_CULL_MODE_FRONT:
		return D3D12_CULL_MODE_FRONT;
	case KOPE_D3D12_CULL_MODE_BACK:
		return D3D12_CULL_MODE_BACK;
	default:
		assert(false);
		return D3D12_CULL_MODE_NONE;
	}
}

D3D12_STENCIL_OP convert_stencil_operation(kope_d3d12_stencil_operation operation) {
	switch (operation) {
	case KOPE_D3D12_STENCIL_OPERATION_KEEP:
		return D3D12_STENCIL_OP_KEEP;
	case KOPE_D3D12_STENCIL_OPERATION_ZERO:
		return D3D12_STENCIL_OP_ZERO;
	case KOPE_D3D12_STENCIL_OPERATION_REPLACE:
		return D3D12_STENCIL_OP_REPLACE;
	case KOPE_D3D12_STENCIL_OPERATION_INVERT:
		return D3D12_STENCIL_OP_INVERT;
	case KOPE_D3D12_STENCIL_OPERATION_INCREMENT_CLAMP:
		return D3D12_STENCIL_OP_INCR_SAT;
	case KOPE_D3D12_STENCIL_OPERATION_DECREMENT_CLAMP:
		return D3D12_STENCIL_OP_DECR_SAT;
	case KOPE_D3D12_STENCIL_OPERATION_INCREMENT_WRAP:
		return D3D12_STENCIL_OP_INCR;
	case KOPE_D3D12_STENCIL_OPERATION_DECREMENT_WRAP:
		return D3D12_STENCIL_OP_DECR;
	default:
		assert(false);
		return D3D12_STENCIL_OP_KEEP;
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE convert_primitive_topology(kope_d3d12_primitive_topology topolocy) {
	switch (topolocy) {
	case KOPE_D3D12_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case KOPE_D3D12_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case KOPE_D3D12_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case KOPE_D3D12_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case KOPE_D3D12_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	default:
		assert(false);
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
}

static void set_blend_state(D3D12_BLEND_DESC *desc, const kope_d3d12_color_target_state *target_state, size_t target) {
	desc->RenderTarget[target].BlendEnable =
	    target_state->blend.color.src_factor != KOPE_D3D12_BLEND_FACTOR_ONE || target_state->blend.color.dst_factor != KOPE_D3D12_BLEND_FACTOR_ZERO ||
	    target_state->blend.alpha.src_factor != KOPE_D3D12_BLEND_FACTOR_ONE || target_state->blend.alpha.dst_factor != KOPE_D3D12_BLEND_FACTOR_ZERO;
	desc->RenderTarget[target].SrcBlend = convert_blend_factor(target_state->blend.color.src_factor);
	desc->RenderTarget[target].DestBlend = convert_blend_factor(target_state->blend.color.dst_factor);
	desc->RenderTarget[target].BlendOp = convert_blend_operation(target_state->blend.color.operation);
	desc->RenderTarget[target].SrcBlendAlpha = convert_blend_factor(target_state->blend.alpha.src_factor);
	desc->RenderTarget[target].DestBlendAlpha = convert_blend_factor(target_state->blend.alpha.dst_factor);
	desc->RenderTarget[target].BlendOpAlpha = convert_blend_operation(target_state->blend.alpha.operation);
	desc->RenderTarget[target].RenderTargetWriteMask = (UINT8)target_state->write_mask;
}

void kope_d3d12_render_pipeline_init(kope_d3d12_device *device, kope_d3d12_render_pipeline *pipe, const kope_d3d12_render_pipeline_parameters *parameters) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {0};

	desc.VS.BytecodeLength = parameters->vertex.shader.size;
	desc.VS.pShaderBytecode = parameters->vertex.shader.data;

	assert(parameters->vertex.buffers_count <= KOPE_D3D12_MAX_VERTEX_ATTRIBUTES);
	D3D12_INPUT_ELEMENT_DESC input_elements[KOPE_D3D12_MAX_VERTEX_ATTRIBUTES] = {0};

	size_t input_element_index = 0;
	for (size_t buffer_index = 0; buffer_index < parameters->vertex.buffers_count; ++buffer_index) {
		for (size_t attribute_index = 0; attribute_index < parameters->vertex.buffers[buffer_index].attributes_count; ++attribute_index) {
			input_elements[input_element_index].SemanticName = "TEXCOORD";
			input_elements[input_element_index].SemanticIndex = parameters->vertex.buffers[buffer_index].attributes[attribute_index].shader_location;
			input_elements[input_element_index].InputSlot = (UINT)buffer_index;
			input_elements[input_element_index].AlignedByteOffset = (attribute_index == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
			input_elements[input_element_index].InputSlotClass = parameters->vertex.buffers[buffer_index].step_mode == KOPE_D3D12_VERTEX_STEP_MODE_INSTANCE
			                                                         ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
			                                                         : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			input_elements[input_element_index].InstanceDataStepRate =
			    parameters->vertex.buffers[buffer_index].step_mode == KOPE_D3D12_VERTEX_STEP_MODE_INSTANCE ? 1 : 0;

			switch (parameters->vertex.buffers[buffer_index].attributes[attribute_index].format) {
			case KOPE_D3D12_VERTEX_FORMAT_UINT8X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT8X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8B8A8_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT8X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT8X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8B8A8_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UNORM8X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8_UNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UNORM8X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SNORM8X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8_SNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SNORM8X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R8G8B8A8_SNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT16X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT16X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16B16A16_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT16X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT16X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16B16A16_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UNORM16X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16_UNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UNORM16X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16B16A16_UNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SNORM16X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16_SNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SNORM16X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT16X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT16X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT32:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT32X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT32X3:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_FLOAT32X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT32:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT32X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT32X3:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UINT32X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32A32_UINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SIN32:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT32X2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT32X3:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_SINT32X4:
				input_elements[input_element_index].Format = DXGI_FORMAT_R32G32B32A32_SINT;
				break;
			case KOPE_D3D12_VERTEX_FORMAT_UNORM10_10_10_2:
				input_elements[input_element_index].Format = DXGI_FORMAT_R10G10B10A2_UNORM;
				break;
			default:
				assert(false);
				break;
			}
			input_element_index += 1;
		}
	}

	desc.InputLayout.NumElements = (UINT)input_element_index;
	desc.InputLayout.pInputElementDescs = input_elements;

	desc.PrimitiveTopologyType = convert_primitive_topology(parameters->primitive.topology);

	desc.RasterizerState.FrontCounterClockwise = parameters->primitive.front_face == KOPE_D3D12_FRONT_FACE_CCW ? TRUE : FALSE;
	desc.RasterizerState.CullMode = convert_cull_mode(parameters->primitive.cull_mode);

	desc.RasterizerState.DepthClipEnable = parameters->primitive.unclipped_depth ? FALSE : TRUE;

	desc.DSVFormat = convert_texture_format(parameters->depth_stencil.format);

	desc.DepthStencilState.DepthEnable = parameters->depth_stencil.depth_compare != KOPE_G5_COMPARE_FUNCTION_ALWAYS;
	desc.DepthStencilState.DepthWriteMask = parameters->depth_stencil.depth_write_enabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	desc.DepthStencilState.DepthFunc = convert_compare_function(parameters->depth_stencil.depth_compare);

	desc.DepthStencilState.StencilEnable = parameters->depth_stencil.stencil_front.compare != KOPE_G5_COMPARE_FUNCTION_ALWAYS ||
	                                       parameters->depth_stencil.stencil_front.pass_op != KOPE_D3D12_STENCIL_OPERATION_KEEP ||
	                                       parameters->depth_stencil.stencil_front.fail_op != KOPE_D3D12_STENCIL_OPERATION_KEEP ||
	                                       parameters->depth_stencil.stencil_front.depth_fail_op != KOPE_D3D12_STENCIL_OPERATION_KEEP ||
	                                       parameters->depth_stencil.stencil_back.compare != KOPE_G5_COMPARE_FUNCTION_ALWAYS ||
	                                       parameters->depth_stencil.stencil_back.pass_op != KOPE_D3D12_STENCIL_OPERATION_KEEP ||
	                                       parameters->depth_stencil.stencil_back.fail_op != KOPE_D3D12_STENCIL_OPERATION_KEEP ||
	                                       parameters->depth_stencil.stencil_back.depth_fail_op != KOPE_D3D12_STENCIL_OPERATION_KEEP;
	desc.DepthStencilState.StencilReadMask = parameters->depth_stencil.stencil_read_mask;
	desc.DepthStencilState.StencilWriteMask = parameters->depth_stencil.stencil_write_mask;
	desc.DepthStencilState.FrontFace.StencilFunc = convert_compare_function(parameters->depth_stencil.stencil_front.compare);
	desc.DepthStencilState.FrontFace.StencilPassOp = convert_stencil_operation(parameters->depth_stencil.stencil_front.pass_op);
	desc.DepthStencilState.FrontFace.StencilFailOp = convert_stencil_operation(parameters->depth_stencil.stencil_front.fail_op);
	desc.DepthStencilState.FrontFace.StencilDepthFailOp = convert_stencil_operation(parameters->depth_stencil.stencil_front.depth_fail_op);
	desc.DepthStencilState.BackFace.StencilFunc = convert_compare_function(parameters->depth_stencil.stencil_back.compare);
	desc.DepthStencilState.BackFace.StencilPassOp = convert_stencil_operation(parameters->depth_stencil.stencil_back.pass_op);
	desc.DepthStencilState.BackFace.StencilFailOp = convert_stencil_operation(parameters->depth_stencil.stencil_back.fail_op);
	desc.DepthStencilState.BackFace.StencilDepthFailOp = convert_stencil_operation(parameters->depth_stencil.stencil_back.depth_fail_op);

	desc.RasterizerState.DepthBias = parameters->depth_stencil.depth_bias;
	desc.RasterizerState.SlopeScaledDepthBias = parameters->depth_stencil.depth_bias_slope_scale;
	desc.RasterizerState.DepthBiasClamp = parameters->depth_stencil.depth_bias_clamp;

	desc.RasterizerState.MultisampleEnable = parameters->multisample.count > 1 ? TRUE : FALSE;
	desc.SampleDesc.Count = parameters->multisample.count;
	desc.SampleDesc.Quality = 0;
	desc.SampleMask = 0xFFFFFFFF;
	desc.BlendState.AlphaToCoverageEnable = parameters->multisample.alpha_to_coverage_enabled ? TRUE : FALSE;

	desc.PS.BytecodeLength = parameters->fragment.shader.size;
	desc.PS.pShaderBytecode = parameters->fragment.shader.data;

	desc.NumRenderTargets = (UINT)parameters->fragment.targets_count;
	assert(parameters->fragment.targets_count <= KOPE_D3D12_MAX_COLOR_TARGETS);
	for (size_t target_index = 0; target_index < parameters->fragment.targets_count; ++target_index) {
		desc.RTVFormats[target_index] = convert_texture_format(parameters->fragment.targets[target_index].format);
	}

	desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	desc.RasterizerState.AntialiasedLineEnable = FALSE;
	desc.RasterizerState.ForcedSampleCount = 0;
	desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	bool independent_blend = false;
	for (int i = 1; i < parameters->fragment.targets_count; ++i) {
		if (memcmp(&parameters->fragment.targets[0], &parameters->fragment.targets[i], sizeof(kope_d3d12_color_target_state)) != 0) {
			independent_blend = true;
			break;
		}
	}

	desc.BlendState.IndependentBlendEnable = independent_blend ? TRUE : FALSE;

	set_blend_state(&desc.BlendState, &parameters->fragment.targets[0], 0);
	if (independent_blend) {
		for (int i = 1; i < parameters->fragment.targets_count; ++i) {
			set_blend_state(&desc.BlendState, &parameters->fragment.targets[i], i);
		}
	}

	desc.pRootSignature = NULL;

	kinc_microsoft_affirm(device->device->CreateGraphicsPipelineState(&desc, IID_GRAPHICS_PPV_ARGS(&pipe->pipe)));

	kinc_microsoft_affirm(
	    device->device->CreateRootSignature(0, desc.VS.pShaderBytecode, desc.VS.BytecodeLength, IID_GRAPHICS_PPV_ARGS(&pipe->root_signature)));
}

void kope_d3d12_render_pipeline_destroy(kope_d3d12_render_pipeline *pipe) {
	if (pipe->pipe != NULL) {
		pipe->pipe->Release();
		pipe->pipe = NULL;
	}
}

void kope_d3d12_compute_pipeline_init(kope_d3d12_device *device, kope_d3d12_compute_pipeline *pipe, const kope_d3d12_compute_pipeline_parameters *parameters) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {0};

	desc.CS.pShaderBytecode = parameters->shader.data;
	desc.CS.BytecodeLength = parameters->shader.size;
	desc.pRootSignature = NULL;

	kinc_microsoft_affirm(device->device->CreateComputePipelineState(&desc, IID_GRAPHICS_PPV_ARGS(&pipe->pipe)));

	kinc_microsoft_affirm(
	    device->device->CreateRootSignature(0, desc.CS.pShaderBytecode, desc.CS.BytecodeLength, IID_GRAPHICS_PPV_ARGS(&pipe->root_signature)));
}

void kope_d3d12_compute_pipeline_destroy(kope_d3d12_compute_pipeline *pipe) {
	if (pipe->pipe != NULL) {
		pipe->pipe->Release();
		pipe->pipe = NULL;
	}
}

#ifndef KONG_HAS_NO_RAY_SHADERS
void kope_d3d12_ray_pipeline_init(kope_g5_device *device, kope_d3d12_ray_pipeline *pipe, const kope_d3d12_ray_pipeline_parameters *parameters,
                                  ID3D12RootSignature *root_signature) {
	D3D12_DXIL_LIBRARY_DESC lib = {0};
	lib.DXILLibrary.pShaderBytecode = ray_code;
	lib.DXILLibrary.BytecodeLength = ray_code_size;

	D3D12_HIT_GROUP_DESC hit_group = {0};
	hit_group.HitGroupExport = L"HitGroup";
	hit_group.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	wchar_t closest_hit[1024];
	kinc_microsoft_convert_string(closest_hit, parameters->closest_shader_name, 1024);
	hit_group.ClosestHitShaderImport = closest_hit;

	wchar_t any_hit[1024];
	wchar_t intersection[1024];

	if (parameters->any_shader_name != NULL) {
		kinc_microsoft_convert_string(any_hit, parameters->any_shader_name, 1024);
		hit_group.AnyHitShaderImport = any_hit;
	}

	if (parameters->intersection_shader_name != NULL) {
		kinc_microsoft_convert_string(intersection, parameters->intersection_shader_name, 1024);
		hit_group.IntersectionShaderImport = intersection;
	}

	D3D12_RAYTRACING_SHADER_CONFIG shader_config = {0};
	shader_config.MaxPayloadSizeInBytes = 20;
	shader_config.MaxAttributeSizeInBytes = 8;

	D3D12_GLOBAL_ROOT_SIGNATURE global_signature = {0};
	global_signature.pGlobalRootSignature = root_signature;

	D3D12_RAYTRACING_PIPELINE_CONFIG pipeline_config = {0};
	pipeline_config.MaxTraceRecursionDepth = 3;

	D3D12_STATE_SUBOBJECT subobjects[5];

	subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subobjects[0].pDesc = &lib;

	subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subobjects[1].pDesc = &hit_group;

	subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subobjects[2].pDesc = &shader_config;

	subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	subobjects[3].pDesc = &global_signature;

	subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subobjects[4].pDesc = &pipeline_config;

	D3D12_STATE_OBJECT_DESC desc;
	desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	desc.NumSubobjects = 5;
	desc.pSubobjects = subobjects;

	kinc_microsoft_affirm(device->d3d12.device->CreateStateObject(&desc, IID_PPV_ARGS(&pipe->pipe)));

	uint32_t shader_id_count = 3;
	kope_g5_buffer_parameters id_buffer_params;
	id_buffer_params.size = shader_id_count * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	id_buffer_params.usage_flags = KOPE_G5_BUFFER_USAGE_CPU_WRITE;
	kope_g5_device_create_buffer(device, &id_buffer_params, &pipe->shader_ids);

	ID3D12StateObjectProperties *props;
	pipe->pipe->QueryInterface(&props);

	uint8_t *data = (uint8_t *)kope_g5_buffer_lock(&pipe->shader_ids);

	wchar_t raygen[1024];
	kinc_microsoft_convert_string(raygen, parameters->gen_shader_name, 1024);
	memcpy(data, props->GetShaderIdentifier(raygen), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	wchar_t miss[1024];
	kinc_microsoft_convert_string(miss, parameters->miss_shader_name, 1024);
	memcpy(&data[D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT], props->GetShaderIdentifier(miss), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	memcpy(&data[D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * 2], props->GetShaderIdentifier(L"HitGroup"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	kope_g5_buffer_unlock(&pipe->shader_ids);

	props->Release();

	pipe->root_signature = root_signature;
}

void kope_d3d12_ray_pipeline_destroy(kope_d3d12_ray_pipeline *pipe) {
	if (pipe->pipe != NULL) {
		pipe->pipe->Release();
		pipe->pipe = NULL;
	}
}
#endif
