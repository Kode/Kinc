#include "pch.h"

#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>

#import <Metal/Metal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

id getMetalDevice();
id getMetalEncoder();

namespace {
	MTLBlendFactor convert(kinc_g5_blending_operation_t op) {
		switch (op) {
			case KINC_G5_BLEND_MODE_ONE:
				return MTLBlendFactorOne;
			case KINC_G5_BLEND_MODE_ZERO:
				return MTLBlendFactorZero;
			case KINC_G5_BLEND_MODE_SOURCE_ALPHA:
				return MTLBlendFactorSourceAlpha;
			case KINC_G5_BLEND_MODE_DEST_ALPHA:
				return MTLBlendFactorDestinationAlpha;
			case KINC_G5_BLEND_MODE_INV_SOURCE_ALPHA:
				return MTLBlendFactorOneMinusSourceAlpha;
			case KINC_G5_BLEND_MODE_INV_DEST_ALPHA:
				return MTLBlendFactorOneMinusDestinationAlpha;
			case KINC_G5_BLEND_MODE_SOURCE_COLOR:
				return MTLBlendFactorSourceColor;
			case KINC_G5_BLEND_MODE_DEST_COLOR:
				return MTLBlendFactorDestinationColor;
			case KINC_G5_BLEND_MODE_INV_SOURCE_COLOR:
				return MTLBlendFactorOneMinusSourceColor;
			case KINC_G5_BLEND_MODE_INV_DEST_COLOR:
				return MTLBlendFactorOneMinusDestinationColor;
		}
	}

	MTLCompareFunction convert(kinc_g5_compare_mode_t compare) {
		switch (compare) {
			case KINC_G5_COMPARE_MODE_ALWAYS:
				return MTLCompareFunctionAlways;
			case KINC_G5_COMPARE_MODE_NEVER:
				return MTLCompareFunctionNever;
			case KINC_G5_COMPARE_MODE_EQUAL:
				return MTLCompareFunctionEqual;
			case KINC_G5_COMPARE_MODE_NOT_EQUAL:
				return MTLCompareFunctionNotEqual;
			case KINC_G5_COMPARE_MODE_LESS:
				return MTLCompareFunctionLess;
			case KINC_G5_COMPARE_MODE_LESS_EQUAL:
				return MTLCompareFunctionLessEqual;
			case KINC_G5_COMPARE_MODE_GREATER:
				return MTLCompareFunctionGreater;
			case KINC_G5_COMPARE_MODE_GREATER_EQUAL:
				return MTLCompareFunctionGreaterEqual;
		}
	}

	MTLCullMode convert(kinc_g5_cull_mode_t cull) {
		switch (cull) {
		case KINC_G5_CULL_MODE_CLOCKWISE:
			return MTLCullModeFront;
		case KINC_G5_CULL_MODE_COUNTERCLOCKWISE:
			return MTLCullModeBack;
		case KINC_G5_CULL_MODE_NEVER:
			return MTLCullModeNone;
		}
	}
}

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipeline) {
	memset(&pipeline->impl, 0, sizeof(pipeline->impl));
}

void kinc_g5_pipeline_destroy(kinc_g5_pipeline_t *pipeline) {
	pipeline->impl._pipeline = 0;
	pipeline->impl._reflection = 0;
	pipeline->impl._depthStencil = 0;
}

static int findAttributeIndex(NSArray<MTLVertexAttribute*>* attributes, const char* name) {
	for (MTLVertexAttribute* attribute in attributes) {
		if (strcmp(name, [[attribute name] UTF8String]) == 0) {
			return (int)[attribute attributeIndex];
		}
	}
	return -1;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipeline) {
	MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = pipeline->vertexShader->impl.mtlFunction;
	renderPipelineDesc.fragmentFunction = pipeline->fragmentShader->impl.mtlFunction;
	renderPipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	renderPipelineDesc.colorAttachments[0].blendingEnabled = pipeline->blendSource != KINC_G5_BLEND_MODE_ONE
		|| pipeline->blendDestination != KINC_G5_BLEND_MODE_ZERO
		|| pipeline->alphaBlendSource != KINC_G5_BLEND_MODE_ONE
		|| pipeline->alphaBlendDestination != KINC_G5_BLEND_MODE_ZERO;
	renderPipelineDesc.colorAttachments[0].sourceRGBBlendFactor = convert(pipeline->blendSource);
	renderPipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = convert(pipeline->alphaBlendSource);
	renderPipelineDesc.colorAttachments[0].destinationRGBBlendFactor = convert(pipeline->blendDestination);
	renderPipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = convert(pipeline->alphaBlendDestination);
	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

	float offset = 0;
	MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; i < pipeline->inputLayout[0]->size; ++i) {
		int index = findAttributeIndex(renderPipelineDesc.vertexFunction.vertexAttributes, pipeline->inputLayout[0]->elements[i].name);

		if (index < 0) {
			fprintf(stderr, "could not find vertex attribute %s\n", pipeline->inputLayout[0]->elements[i].name);
		}

		vertexDescriptor.attributes[index].bufferIndex = 0;
		vertexDescriptor.attributes[index].offset = offset;

		switch (pipeline->inputLayout[0]->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat;
			offset += sizeof(float);
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat2;
			offset += 2 * sizeof(float);
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat3;
			offset += 3 * sizeof(float);
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat4;
			offset += 4 * sizeof(float);
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			vertexDescriptor.attributes[index].format = MTLVertexFormatShort2Normalized;
			offset += 2 * sizeof(short);
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			vertexDescriptor.attributes[index].format = MTLVertexFormatShort4Normalized;
			offset += 4 * sizeof(short);
			break;
		default:
			break;
		}
	}

	vertexDescriptor.layouts[0].stride = offset;
	vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

	renderPipelineDesc.vertexDescriptor = vertexDescriptor;

	NSError* errors = nil;
	MTLRenderPipelineReflection* reflection = nil;
	id<MTLDevice> device = getMetalDevice();
	pipeline->impl._pipeline = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&errors];
	if (errors != nil) NSLog(@"%@", [errors localizedDescription]);
	assert(pipeline->impl._pipeline && !errors);
	pipeline->impl._reflection = reflection;

	MTLDepthStencilDescriptor* depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	depthStencilDescriptor.depthCompareFunction = convert(pipeline->depthMode);
	depthStencilDescriptor.depthWriteEnabled = pipeline->depthWrite;
	pipeline->impl._depthStencil = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

void kinc_g5_internal_pipeline_set(kinc_g5_pipeline_t *pipeline) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setRenderPipelineState:pipeline->impl._pipeline];
	[encoder setDepthStencilState:pipeline->impl._depthStencil];
	[encoder setFrontFacingWinding:MTLWindingClockwise];
	[encoder setCullMode:convert(pipeline->cullMode)];
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;

	MTLRenderPipelineReflection* reflection = pipeline->impl._reflection;

	for (MTLArgument* arg in reflection.vertexArguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.vertexOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	for (MTLArgument* arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeBuffer && [[arg name] isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.fragmentOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipeline, const char *name) {
	kinc_g5_texture_unit_t unit;
	unit.impl.index = -1;

	MTLRenderPipelineReflection* reflection = pipeline->impl._reflection;
	for (MTLArgument* arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.impl.index = (int)[arg index];
		}
	}

	return unit;
}
