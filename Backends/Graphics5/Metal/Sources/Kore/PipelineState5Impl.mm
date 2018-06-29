#include "pch.h"

#include <Kore/Graphics5/PipelineState.h>
#include <Kore/Graphics5/Shader.h>

#import <Metal/Metal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

namespace {
	MTLBlendFactor convert(Kore::Graphics5::BlendingOperation op) {
		switch (op) {
			case Graphics5::BlendOne:
				return MTLBlendFactorOne;
			case Graphics5::BlendZero:
				return MTLBlendFactorZero;
			case Graphics5::SourceAlpha:
				return MTLBlendFactorSourceAlpha;
			case Graphics5::DestinationAlpha:
				return MTLBlendFactorDestinationAlpha;
			case Graphics5::InverseSourceAlpha:
				return MTLBlendFactorOneMinusSourceAlpha;
			case Graphics5::InverseDestinationAlpha:
				return MTLBlendFactorOneMinusDestinationAlpha;
			case Graphics5::SourceColor:
				return MTLBlendFactorSourceColor;
			case Graphics5::DestinationColor:
				return MTLBlendFactorDestinationColor;
			case Graphics5::InverseSourceColor:
				return MTLBlendFactorOneMinusSourceColor;
			case Graphics5::InverseDestinationColor:
				return MTLBlendFactorOneMinusDestinationColor;
		}
	}
	
	MTLCompareFunction convert(Kore::Graphics5::ZCompareMode compare) {
		switch (compare) {
			case Graphics5::ZCompareAlways:
				return MTLCompareFunctionAlways;
			case Graphics5::ZCompareNever:
				return MTLCompareFunctionNever;
			case Graphics5::ZCompareEqual:
				return MTLCompareFunctionEqual;
			case Graphics5::ZCompareNotEqual:
				return MTLCompareFunctionNotEqual;
			case Graphics5::ZCompareLess:
				return MTLCompareFunctionLess;
			case Graphics5::ZCompareLessEqual:
				return MTLCompareFunctionLessEqual;
			case Graphics5::ZCompareGreater:
				return MTLCompareFunctionGreater;
			case Graphics5::ZCompareGreaterEqual:
				return MTLCompareFunctionGreaterEqual;
		}
	}
}

PipelineState5Impl::PipelineState5Impl() : _pipeline(nullptr) {
	
}

PipelineState5Impl::~PipelineState5Impl() {

}

static int findAttributeIndex(NSArray<MTLVertexAttribute*>* attributes, const char* name) {
	for (MTLVertexAttribute* attribute in attributes) {
		if (strcmp(name, [[attribute name] UTF8String]) == 0) {
			return (int)[attribute attributeIndex];
		}
	}
	return -1;
}

void Graphics5::PipelineState::compile() {
	MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = vertexShader->mtlFunction;
	renderPipelineDesc.fragmentFunction = fragmentShader->mtlFunction;
	renderPipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	renderPipelineDesc.colorAttachments[0].blendingEnabled = blendSource != BlendOne || blendDestination != BlendZero || alphaBlendSource != BlendOne || alphaBlendDestination != BlendZero;
	renderPipelineDesc.colorAttachments[0].sourceRGBBlendFactor = convert(blendSource);
	renderPipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = convert(alphaBlendSource);
	renderPipelineDesc.colorAttachments[0].destinationRGBBlendFactor = convert(blendDestination);
	renderPipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = convert(alphaBlendDestination);
#ifdef KORE_IOS
	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
#else
	renderPipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
	renderPipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
#endif
	
	float offset = 0;
	MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; i < inputLayout[0]->size; ++i) {
		int index = findAttributeIndex(renderPipelineDesc.vertexFunction.vertexAttributes, inputLayout[0]->elements[i].name);
		
		vertexDescriptor.attributes[index].bufferIndex = 0;
		vertexDescriptor.attributes[index].offset = offset;

		switch (inputLayout[0]->elements[i].data) {
		case Graphics4::Float1VertexData:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat;
			offset += sizeof(float);
			break;
		case Graphics4::Float2VertexData:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat2;
			offset += 2 * sizeof(float);
			break;
		case Graphics4::Float3VertexData:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat3;
			offset += 3 * sizeof(float);
			break;
		case Graphics4::Float4VertexData:
			vertexDescriptor.attributes[index].format = MTLVertexFormatFloat4;
			offset += 4 * sizeof(float);
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
	_pipeline = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&errors];
	if (errors != nil) NSLog(@"%@", [errors localizedDescription]);
	assert(_pipeline && !errors);
	_reflection = reflection;
	
	MTLDepthStencilDescriptor* depthStencilDescriptor = [MTLDepthStencilDescriptor new];
	depthStencilDescriptor.depthCompareFunction = convert(depthMode);
	depthStencilDescriptor.depthWriteEnabled = depthWrite;
	_depthStencil = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

void PipelineState5Impl::_set() {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setRenderPipelineState:_pipeline];
	[encoder setDepthStencilState:_depthStencil];
	//[encoder setFrontFacingWinding:MTLWindingCounterClockwise];
	//[encoder setCullMode:MTLCullModeBack];
}

Graphics5::ConstantLocation Graphics5::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.vertexOffset = -1;
	location.fragmentOffset = -1;

	MTLRenderPipelineReflection* reflection = _reflection;

	for (MTLArgument* arg in reflection.vertexArguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.vertexOffset = (int)[member offset];
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
						location.fragmentOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

Graphics5::TextureUnit Graphics5::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit.index = -1;

	MTLRenderPipelineReflection* reflection = _reflection;
	for (MTLArgument* arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.index = (int)[arg index];
		}
	}

	return unit;
}
