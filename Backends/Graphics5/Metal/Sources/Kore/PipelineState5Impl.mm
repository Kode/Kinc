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

PipelineState5Impl::PipelineState5Impl() {}

void Graphics5::PipelineState::compile() {
	MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = vertexShader->mtlFunction;
	renderPipelineDesc.fragmentFunction = fragmentShader->mtlFunction;
	renderPipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
	// Create a vertex descriptor
	float offset = 0;
	MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];

	for (int i = 0; inputLayout[i] != nullptr; ++i) {
		
		vertexDescriptor.attributes[i].bufferIndex = 0;
		vertexDescriptor.attributes[i].offset = offset;
		
		switch (inputLayout[0]->elements[i].data) {
			case Graphics4::Float1VertexData:
				vertexDescriptor.attributes[i].format = MTLVertexFormatFloat;
				offset += sizeof(float);
				break;
			case Graphics4::Float2VertexData:
				vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;
				offset += 2 * sizeof(float);
				break;
			case Graphics4::Float3VertexData:
				vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;
				offset += 3 * sizeof(float);
				break;
			case Graphics4::Float4VertexData:
				vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;
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
	pipeline = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&errors];
	if (errors != nil) NSLog(@"%@",[errors localizedDescription]);
	assert(pipeline && !errors);
	this->reflection = reflection;
}

void PipelineState5Impl::_set() {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setRenderPipelineState:pipeline];
}

Graphics5::ConstantLocation Graphics5::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.vertexOffset = -1;
	location.fragmentOffset = -1;

	MTLRenderPipelineReflection* reflection = this->reflection;

	for (MTLArgument* arg in reflection.vertexArguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.vertexOffset = (int)[member offset];
					}
				}
			}
		}
	}

	for (MTLArgument* arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeBuffer && [[arg name] isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.vertexOffset = (int)[member offset];
					}
				}
			}
		}
	}

	return location;
}

Graphics5::TextureUnit Graphics5::PipelineState::getTextureUnit(const char* name) {
	TextureUnit unit;
	unit.index = -1;

	MTLRenderPipelineReflection* reflection = this->reflection;
	for (MTLArgument* arg in reflection.fragmentArguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.index = (int)[arg index];
		}
	}

	return unit;
}
