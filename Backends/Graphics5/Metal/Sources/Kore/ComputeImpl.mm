#include "pch.h"

#include "ComputeImpl.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Math/Core.h>

#include <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();
id getMetalLibrary();

namespace {
	const int constantsSize = 1024 * 4;
	u8* constantsMemory;

	void setFloat(u8* constants, u32 offset, u32 size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, u32 offset, u32 size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, u32 offset, u32 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, u32 offset, u32 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}
	
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLComputeCommandEncoder> commandEncoder;
	id<MTLBuffer> buffer;
}

void initMetalCompute(id<MTLDevice> device, id<MTLCommandQueue> queue) {
	commandQueue = queue;
	commandBuffer = [commandQueue commandBuffer];
	commandEncoder = [commandBuffer computeCommandEncoder];
	buffer = [device newBufferWithLength:constantsSize options:MTLResourceOptionCPUCacheModeDefault];
	constantsMemory = (u8*)[buffer contents];
}

void shutdownMetalCompute() {
	[commandEncoder endEncoding];
	commandEncoder = nil;
	commandBuffer = nil;
	commandQueue = nil;
}

ComputeShaderImpl::ComputeShaderImpl(void* source, int length) {
	u8* data = (u8*)source;
	for (int i = 0; i < length; ++i) {
		name[i] = data[i];
	}
	name[length] = 0;
}

ComputeShader::ComputeShader(void* _data, int length) : ComputeShaderImpl(_data, length) {
	id<MTLLibrary> library = getMetalLibrary();
	_function = [library newFunctionWithName:[NSString stringWithCString:name encoding:NSUTF8StringEncoding]];
	assert(_function);

	id<MTLDevice> device = getMetalDevice();
	MTLComputePipelineReflection* reflection = nil;
	NSError* error = nil;
	_pipeline = [device newComputePipelineStateWithFunction:_function options:MTLPipelineOptionBufferTypeInfo reflection:&reflection error:&error];
	if (error != nil) NSLog(@"%@", [error localizedDescription]);
	assert(_pipeline && !error);
	_reflection = reflection;
}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	location._offset = -1;
	
	MTLComputePipelineReflection* reflection = _reflection;
	
	for (MTLArgument* arg in reflection.arguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType* structObj = [arg bufferStructType];
				for (MTLStructMember* member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location._offset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}
	
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	unit._index = -1;
	
	MTLComputePipelineReflection* reflection = _reflection;
	for (MTLArgument* arg in reflection.arguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit._index = (int)[arg index];
		}
	}
	
	return unit;
}

void Compute::setBool(ComputeConstantLocation location, bool value) {}

void Compute::setInt(ComputeConstantLocation location, int value) {}

void Compute::setFloat(ComputeConstantLocation location, float value) {
	::setFloat(constantsMemory, location._offset, 4, value);
}

void Compute::setFloat2(ComputeConstantLocation location, float value1, float value2) {
	::setFloat2(constantsMemory, location._offset, 4 * 2, value1, value2);
}

void Compute::setFloat3(ComputeConstantLocation location, float value1, float value2, float value3) {
	::setFloat3(constantsMemory, location._offset, 4 * 3, value1, value2, value3);
}

void Compute::setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4) {
	::setFloat4(constantsMemory, location._offset, 4 * 4, value1, value2, value3, value4);
}

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat3& value) {}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {
	[commandEncoder setTexture:texture->_texture->_tex atIndex:unit._index];
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target, Access access) {}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture) {}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {}

void Compute::setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {}

void Compute::setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {}

void Compute::setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {}

void Compute::setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {}

void Compute::setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {}

void Compute::setShader(ComputeShader* shader) {
	[commandEncoder setComputePipelineState:shader->_pipeline];
}

void Compute::compute(int x, int y, int z) {
	[commandEncoder setBuffer:buffer offset:0 atIndex:0];
	
	MTLSize perGrid;
	perGrid.width = x;
	perGrid.height = y;
	perGrid.depth = z;
	MTLSize perGroup;
	perGroup.width = 16;
	perGroup.height = 16;
	perGroup.depth = 1;
	[commandEncoder dispatchThreadgroups:perGrid threadsPerThreadgroup:perGroup];
	
	[commandEncoder endEncoding];
	[commandBuffer commit];
	[commandBuffer waitUntilCompleted];
	
	commandBuffer = [commandQueue commandBuffer];
	commandEncoder = [commandBuffer computeCommandEncoder];
}
