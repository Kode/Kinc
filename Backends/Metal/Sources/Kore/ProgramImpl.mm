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

void Program::link(const VertexStructure& structure) {

}

void Program::set() {
	MTLRenderPipelineDescriptor* renderPipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
	renderPipelineDesc.vertexFunction = vertexShader->mtlFunction;
	renderPipelineDesc.fragmentFunction = fragmentShader->mtlFunction;
	renderPipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
 
	NSError* errors = nil;
	id <MTLDevice> device = getMetalDevice();
	id <MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:renderPipelineDesc error:&errors];
	assert(pipeline && !errors);
 
	id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setRenderPipelineState:pipeline];
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location;
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit;
	return unit;
}
