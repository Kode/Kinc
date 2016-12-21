#include "pch.h"

#include <Kore/Graphics/Shader.h>

#import <Metal/Metal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

ProgramImpl::ProgramImpl() {}

Program::Program() {}

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
    
    // Create a vertex descriptor
    float offset = 0;
    MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    
    for (int i = 0; i < structures[0]->size; ++i) {
        
        vertexDescriptor.attributes[i].bufferIndex = 0;
        vertexDescriptor.attributes[i].offset = offset;
        
        switch (structures[0]->elements[i].data) {
            case Float1VertexData:
                vertexDescriptor.attributes[i].format = MTLVertexFormatFloat;
                offset += sizeof(float);
                break;
            case Float2VertexData:
                vertexDescriptor.attributes[i].format = MTLVertexFormatFloat2;
                offset += 2 * sizeof(float);
                break;
            case Float3VertexData:
                vertexDescriptor.attributes[i].format = MTLVertexFormatFloat3;
                offset += 3 * sizeof(float);
                break;
            case Float4VertexData:
                vertexDescriptor.attributes[i].format = MTLVertexFormatFloat4;
                offset += 4 * sizeof(float);
                break;
            case Float4x4VertexData:
                // TODO
                break;
            case ColorVertexData:
                // TODO
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
    NSLog(@"%@",[errors localizedDescription]);
    assert(pipeline && !errors);
	this->reflection = reflection;
}

void Program::set() {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
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
