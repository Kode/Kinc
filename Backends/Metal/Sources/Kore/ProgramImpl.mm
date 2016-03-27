#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

ProgramImpl::ProgramImpl() {

}

Program::Program() {
	
}
	
void Program::setVertexShader(Shader* vertexShader) {
	this->vertexShader = vertexShader;
}
	
void Program::setFragmentShader(Shader* fragmentShader) {
	this->fragmentShader = fragmentShader;
}

void Program::link(VertexStructure** structures, int count) {
	MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = vertexShader->mtlFunction;
	renderPipelineDesc.fragmentFunction = fragmentShader->mtlFunction;
	renderPipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
 
	NSError* errors = nil;
	MTLRenderPipelineReflection* reflection = nil;
	id <MTLDevice> device = getMetalDevice();
	pipeline = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&errors];
	assert(pipeline && !errors);
	this->reflection = reflection;
	
	
}

void Program::set() {
	id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setRenderPipelineState:pipeline];
}

ConstantLocation Program::getConstantLocation(const char* name) {
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

TextureUnit Program::getTextureUnit(const char* name) {
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
