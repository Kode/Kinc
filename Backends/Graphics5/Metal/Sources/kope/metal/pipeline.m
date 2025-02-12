#include "pipeline_functions.h"
#include "pipeline_structs.h"

#include "metalunit.h"

#include <kinc/log.h>

static MTLBlendFactor convert_blending_factor(kope_metal_blend_factor factor) {
	switch (factor) {
	case KOPE_METAL_BLEND_FACTOR_ONE:
		return MTLBlendFactorOne;
	case KOPE_METAL_BLEND_FACTOR_ZERO:
		return MTLBlendFactorZero;
	case KOPE_METAL_BLEND_FACTOR_SRC_ALPHA:
		return MTLBlendFactorSourceAlpha;
	case KOPE_METAL_BLEND_FACTOR_DST_ALPHA:
		return MTLBlendFactorDestinationAlpha;
	case KOPE_METAL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		return MTLBlendFactorOneMinusSourceAlpha;
	case KOPE_METAL_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		return MTLBlendFactorOneMinusDestinationAlpha;
	case KOPE_METAL_BLEND_FACTOR_SRC:
		return MTLBlendFactorSourceColor;
	case KOPE_METAL_BLEND_FACTOR_DST:
		return MTLBlendFactorDestinationColor;
	case KOPE_METAL_BLEND_FACTOR_ONE_MINUS_SRC:
		return MTLBlendFactorOneMinusSourceColor;
	case KOPE_METAL_BLEND_FACTOR_ONE_MINUS_DST:
		return MTLBlendFactorOneMinusDestinationColor;
	case KOPE_METAL_BLEND_FACTOR_CONSTANT:
		return MTLBlendFactorBlendColor;
	case KOPE_METAL_BLEND_FACTOR_ONE_MINUS_CONSTANT:
		return MTLBlendFactorOneMinusBlendColor;
	case KOPE_METAL_BLEND_FACTOR_SRC_ALPHA_SATURATED:
		return MTLBlendFactorSourceAlphaSaturated;
	}
}

static MTLBlendOperation convert_blending_operation(kope_metal_blend_operation op) {
	switch (op) {
	case KOPE_METAL_BLEND_OPERATION_ADD:
		return MTLBlendOperationAdd;
	case KOPE_METAL_BLEND_OPERATION_SUBTRACT:
		return MTLBlendOperationSubtract;
	case KOPE_METAL_BLEND_OPERATION_REVERSE_SUBTRACT:
		return MTLBlendOperationReverseSubtract;
	case KOPE_METAL_BLEND_OPERATION_MIN:
		return MTLBlendOperationMin;
	case KOPE_METAL_BLEND_OPERATION_MAX:
		return MTLBlendOperationMax;
	}
}

static MTLPixelFormat convert_format(kope_g5_texture_format format) {
	switch (format) {
		case KOPE_G5_TEXTURE_FORMAT_R8_UNORM:
			return MTLPixelFormatR8Unorm;
		case KOPE_G5_TEXTURE_FORMAT_R8_SNORM:
			return MTLPixelFormatR8Snorm;
		case KOPE_G5_TEXTURE_FORMAT_R8_UINT:
			return MTLPixelFormatR8Uint;
		case KOPE_G5_TEXTURE_FORMAT_R8_SINT:
			return MTLPixelFormatR8Sint;
		case KOPE_G5_TEXTURE_FORMAT_R16_UINT:
			return MTLPixelFormatR16Uint;
		case KOPE_G5_TEXTURE_FORMAT_R16_SINT:
			return MTLPixelFormatR16Sint;
		case KOPE_G5_TEXTURE_FORMAT_R16_FLOAT:
			return MTLPixelFormatR16Float;
		case KOPE_G5_TEXTURE_FORMAT_RG8_UNORM:
			return MTLPixelFormatRG8Unorm;
		case KOPE_G5_TEXTURE_FORMAT_RG8_SNORM:
			return MTLPixelFormatRG8Snorm;
		case KOPE_G5_TEXTURE_FORMAT_RG8_UINT:
			return MTLPixelFormatRG8Uint;
		case KOPE_G5_TEXTURE_FORMAT_RG8_SINT:
			return MTLPixelFormatRG8Sint;
		case KOPE_G5_TEXTURE_FORMAT_R32_UINT:
			return MTLPixelFormatR32Uint;
		case KOPE_G5_TEXTURE_FORMAT_R32_SINT:
			return MTLPixelFormatR32Sint;
		case KOPE_G5_TEXTURE_FORMAT_R32_FLOAT:
			return MTLPixelFormatR32Float;
		case KOPE_G5_TEXTURE_FORMAT_RG16_UINT:
			return MTLPixelFormatRG16Uint;
		case KOPE_G5_TEXTURE_FORMAT_RG16_SINT:
			return MTLPixelFormatRG16Sint;
		case KOPE_G5_TEXTURE_FORMAT_RG16_FLOAT:
			return MTLPixelFormatRG16Float;
		case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM:
			return MTLPixelFormatRGBA8Unorm;
		case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM_SRGB:
			return MTLPixelFormatRGBA8Unorm_sRGB;
		case KOPE_G5_TEXTURE_FORMAT_RGBA8_SNORM:
			return MTLPixelFormatRGBA8Unorm;
		case KOPE_G5_TEXTURE_FORMAT_RGBA8_UINT:
			return MTLPixelFormatRGBA8Uint;
		case KOPE_G5_TEXTURE_FORMAT_RGBA8_SINT:
			return MTLPixelFormatRGBA8Sint;
		case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM:
			return MTLPixelFormatBGRA8Unorm;
		case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
			return MTLPixelFormatBGRA8Unorm_sRGB;
		case KOPE_G5_TEXTURE_FORMAT_RGB9E5U_FLOAT:
			return MTLPixelFormatRGB9E5Float;
		case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UINT:
			return MTLPixelFormatRGB10A2Uint;
		case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UNORM:
			return MTLPixelFormatRGB10A2Unorm;
		case KOPE_G5_TEXTURE_FORMAT_RG11B10U_FLOAT:
			return MTLPixelFormatRG11B10Float;
		case KOPE_G5_TEXTURE_FORMAT_RG32_UINT:
			return MTLPixelFormatRG32Uint;
		case KOPE_G5_TEXTURE_FORMAT_RG32_SINT:
			return MTLPixelFormatRG32Sint;
		case KOPE_G5_TEXTURE_FORMAT_RG32_FLOAT:
			return MTLPixelFormatRG32Float;
		case KOPE_G5_TEXTURE_FORMAT_RGBA16_UINT:
			return MTLPixelFormatRGBA16Uint;
		case KOPE_G5_TEXTURE_FORMAT_RGBA16_SINT:
			return MTLPixelFormatRGBA16Sint;
		case KOPE_G5_TEXTURE_FORMAT_RGBA16_FLOAT:
			return MTLPixelFormatRGBA16Float;
		case KOPE_G5_TEXTURE_FORMAT_RGBA32_UINT:
			return MTLPixelFormatRGBA32Uint;
		case KOPE_G5_TEXTURE_FORMAT_RGBA32_SINT:
			return MTLPixelFormatRGBA32Sint;
		case KOPE_G5_TEXTURE_FORMAT_RGBA32_FLOAT:
			return MTLPixelFormatRGBA32Float;
		case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
			return MTLPixelFormatDepth16Unorm;
		case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
		case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
			return MTLPixelFormatDepth24Unorm_Stencil8;
		case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
			return MTLPixelFormatDepth32Float;
		case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
			return MTLPixelFormatDepth32Float_Stencil8;
	}
}

static uint32_t vertex_attribute_size(kope_metal_vertex_format format) {
	switch (format) {
	case KOPE_METAL_VERTEX_FORMAT_UINT8X2:
		return 2;
	case KOPE_METAL_VERTEX_FORMAT_UINT8X4:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_SINT8X2:
		return 2;
	case KOPE_METAL_VERTEX_FORMAT_SINT8X4:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_UNORM8X2:
		return 2;
	case KOPE_METAL_VERTEX_FORMAT_UNORM8X4:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_SNORM8X2:
		return 2;
	case KOPE_METAL_VERTEX_FORMAT_SNORM8X4:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_UINT16X2:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_UINT16X4:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_SINT16X2:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_SINT16X4:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_UNORM16X2:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_UNORM16X4:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_SNORM16X2:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_SNORM16X4:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT16X2:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT16X4:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT32:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT32X2:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT32X3:
		return 12;
	case KOPE_METAL_VERTEX_FORMAT_FLOAT32X4:
		return 16;
	case KOPE_METAL_VERTEX_FORMAT_UINT32:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_UINT32X2:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_UINT32X3:
		return 12;
	case KOPE_METAL_VERTEX_FORMAT_UINT32X4:
		return 16;
	case KOPE_METAL_VERTEX_FORMAT_SINT32:
		return 4;
	case KOPE_METAL_VERTEX_FORMAT_SINT32X2:
		return 8;
	case KOPE_METAL_VERTEX_FORMAT_SINT32X3:
		return 12;
	case KOPE_METAL_VERTEX_FORMAT_SINT32X4:
		return 16;
	case KOPE_METAL_VERTEX_FORMAT_UNORM10_10_10_2:
		return 4;
	}

	return 0;
}


void kope_metal_render_pipeline_init(kope_metal_device *device, kope_metal_render_pipeline *pipe, const kope_metal_render_pipeline_parameters *parameters) {
	id<MTLLibrary> library = (__bridge id<MTLLibrary>)device->library;

	id vertex_function = [library newFunctionWithName:[NSString stringWithCString:parameters->vertex.shader.function_name encoding:NSUTF8StringEncoding]];
	id fragment_function = [library newFunctionWithName:[NSString stringWithCString:parameters->fragment.shader.function_name encoding:NSUTF8StringEncoding]];
	
	MTLRenderPipelineDescriptor *render_pipeline_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
	render_pipeline_descriptor.vertexFunction = vertex_function;
	render_pipeline_descriptor.fragmentFunction = fragment_function;
	
	for (int i = 0; i < parameters->fragment.targets_count; ++i) {
		render_pipeline_descriptor.colorAttachments[i].pixelFormat = convert_format(parameters->fragment.targets[i].format);
		
		render_pipeline_descriptor.colorAttachments[i].blendingEnabled =
			parameters->fragment.targets[i].blend.color.src_factor != KOPE_METAL_BLEND_FACTOR_ONE || parameters->fragment.targets[i].blend.color.dst_factor != KOPE_METAL_BLEND_FACTOR_ZERO ||
			parameters->fragment.targets[i].blend.alpha.src_factor != KOPE_METAL_BLEND_FACTOR_ONE || parameters->fragment.targets[i].blend.alpha.dst_factor != KOPE_METAL_BLEND_FACTOR_ZERO;
		
		render_pipeline_descriptor.colorAttachments[i].sourceRGBBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.color.src_factor);
		render_pipeline_descriptor.colorAttachments[i].destinationRGBBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.color.dst_factor);
		render_pipeline_descriptor.colorAttachments[i].rgbBlendOperation = convert_blending_operation(parameters->fragment.targets[i].blend.color.operation);
		
		render_pipeline_descriptor.colorAttachments[i].sourceAlphaBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.alpha.src_factor);
		render_pipeline_descriptor.colorAttachments[i].destinationAlphaBlendFactor = convert_blending_factor(parameters->fragment.targets[i].blend.alpha.dst_factor);
		render_pipeline_descriptor.colorAttachments[i].alphaBlendOperation = convert_blending_operation(parameters->fragment.targets[i].blend.alpha.operation);
		
		render_pipeline_descriptor.colorAttachments[i].writeMask = parameters->fragment.targets[i].write_mask;
	}
	render_pipeline_descriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
	render_pipeline_descriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
	
	float offset = 0;
	MTLVertexDescriptor *vertex_descriptor = [[MTLVertexDescriptor alloc] init];
	
	uint32_t attributes_count = 0;
	uint32_t bindings_count = 0;
	for (int i = 0; i < parameters->vertex.buffers_count; ++i) {
		attributes_count += (uint32_t)parameters->vertex.buffers[i].attributes_count;
		++bindings_count;
	}

	for (uint32_t binding_index = 0; binding_index < bindings_count; ++binding_index) {
		for (uint32_t attribute_index = 0; attribute_index < parameters->vertex.buffers[binding_index].attributes_count; ++attribute_index) {
			kope_metal_vertex_attribute attribute = parameters->vertex.buffers[binding_index].attributes[attribute_index];
			
			vertex_descriptor.attributes[attribute_index].bufferIndex = 0;
			vertex_descriptor.attributes[attribute_index].offset = offset;
			
			offset += vertex_attribute_size(attribute.format);
			
			switch (attribute.format) {
				case KOPE_METAL_VERTEX_FORMAT_FLOAT32:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatFloat;
					break;
				case KOPE_METAL_VERTEX_FORMAT_FLOAT32X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatFloat2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_FLOAT32X3:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatFloat3;
					break;
				case KOPE_METAL_VERTEX_FORMAT_FLOAT32X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatFloat4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT8X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatChar2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT8X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUChar2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SNORM8X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatChar2Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UNORM8X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUChar2Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT8X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatChar4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT8X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUChar4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SNORM8X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatChar4Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UNORM8X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUChar4Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT16X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatShort2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT16X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUShort2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SNORM16X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatShort2Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UNORM16X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUShort2Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT16X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatShort4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT16X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUShort4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SNORM16X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatShort4Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UNORM16X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUShort4Normalized;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT32:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatInt;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT32:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUInt;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT32X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatInt2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT32X2:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUInt2;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT32X3:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatInt3;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT32X3:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUInt3;
					break;
				case KOPE_METAL_VERTEX_FORMAT_SINT32X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatInt4;
					break;
				case KOPE_METAL_VERTEX_FORMAT_UINT32X4:
					vertex_descriptor.attributes[attribute_index].format = MTLVertexFormatUInt4;
					break;
				default:
					assert(false);
					break;
			}
		}
	}

	vertex_descriptor.layouts[0].stride = offset;
	vertex_descriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	render_pipeline_descriptor.vertexDescriptor = vertex_descriptor;
	
	NSError *errors = nil;
	MTLRenderPipelineReflection *reflection = nil;
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->device;

	pipe->pipeline = (__bridge_retained void *)[metal_device newRenderPipelineStateWithDescriptor:render_pipeline_descriptor
																							  options:MTLPipelineOptionBufferTypeInfo
																						   reflection:&reflection
																								error:&errors];
}

void kope_metal_render_pipeline_destroy(kope_metal_render_pipeline *pipe) {}

void kope_metal_compute_pipeline_init(kope_metal_device *device, kope_metal_compute_pipeline *pipe, const kope_metal_compute_pipeline_parameters *parameters) {}

void kope_metal_compute_pipeline_destroy(kope_metal_compute_pipeline *pipe) {}

void kope_metal_ray_pipeline_init(kope_g5_device *device, kope_metal_ray_pipeline *pipe, const kope_metal_ray_pipeline_parameters *parameters) {}

void kope_metal_ray_pipeline_destroy(kope_metal_ray_pipeline *pipe) {}
