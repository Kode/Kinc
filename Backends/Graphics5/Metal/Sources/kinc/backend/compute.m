#include <kinc/compute/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/math/core.h>

#include <Metal/Metal.h>

id getMetalDevice(void);
id getMetalLibrary(void);

#define constantsSize 1024 * 4
static uint8_t *constantsMemory;

static void setFloat(uint8_t *constants, uint32_t offset, uint32_t size, float value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value;
}

static void setFloat2(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
}

static void setFloat3(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

static void setFloat4(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3, float value4) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

static id<MTLCommandQueue> commandQueue;
static id<MTLCommandBuffer> commandBuffer;
static id<MTLComputeCommandEncoder> commandEncoder;
static id<MTLBuffer> buffer;

void initMetalCompute(id<MTLDevice> device, id<MTLCommandQueue> queue) {
	commandQueue = queue;
	commandBuffer = [commandQueue commandBuffer];
	commandEncoder = [commandBuffer computeCommandEncoder];
	buffer = [device newBufferWithLength:constantsSize options:MTLResourceOptionCPUCacheModeDefault];
	constantsMemory = (uint8_t *)[buffer contents];
}

void shutdownMetalCompute(void) {
	[commandEncoder endEncoding];
	commandEncoder = nil;
	commandBuffer = nil;
	commandQueue = nil;
}

void kinc_compute_shader_destroy(kinc_compute_shader_t *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl._function;
	function = nil;
	shader->impl._function = NULL;

	id<MTLComputePipelineState> pipeline = (__bridge_transfer id<MTLComputePipelineState>)shader->impl._pipeline;
	pipeline = nil;
	shader->impl._pipeline = NULL;

	MTLComputePipelineReflection *reflection = (__bridge_transfer MTLComputePipelineReflection *)shader->impl._reflection;
	reflection = nil;
	shader->impl._reflection = NULL;
}

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *_data, int length) {
	shader->impl.name[0] = 0;

	{
		uint8_t *data = (uint8_t *)_data;
		if (length > 1 && data[0] == '>') {
			memcpy(shader->impl.name, data + 1, length - 1);
			shader->impl.name[length - 1] = 0;
		}
		else {
			for (int i = 3; i < length; ++i) {
				if (data[i] == '\n') {
					shader->impl.name[i - 3] = 0;
					break;
				}
				else {
					shader->impl.name[i - 3] = data[i];
				}
			}
		}
	}

	char *data = (char *)_data;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:nil];
	}
	id<MTLFunction> function = [library newFunctionWithName:[NSString stringWithCString:shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(shader->impl._function != nil);
	shader->impl._function = (__bridge_retained void *)function;

	id<MTLDevice> device = getMetalDevice();
	MTLComputePipelineReflection *reflection = nil;
	NSError *error = nil;
	shader->impl._pipeline = (__bridge_retained void *)[device newComputePipelineStateWithFunction:function
	                                                                                       options:MTLPipelineOptionBufferTypeInfo
	                                                                                    reflection:&reflection
	                                                                                         error:&error];
	if (error != nil)
		NSLog(@"%@", [error localizedDescription]);
	assert(shader->impl._pipeline != NULL && !error);
	shader->impl._reflection = (__bridge_retained void *)reflection;
}

kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_constant_location_t location;
	location.impl._offset = -1;

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;

	for (MTLArgument *arg in reflection.arguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl._offset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_texture_unit_t unit;
	unit.impl._index = -1;

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;
	for (MTLArgument *arg in reflection.arguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.impl._index = (int)[arg index];
		}
	}

	return unit;
}

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value) {}

void kinc_compute_set_int(kinc_compute_constant_location_t location, int value) {}

void kinc_compute_set_float(kinc_compute_constant_location_t location, float value) {
	setFloat(constantsMemory, location.impl._offset, 4, value);
}

void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2) {
	setFloat2(constantsMemory, location.impl._offset, 4 * 2, value1, value2);
}

void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3) {
	setFloat3(constantsMemory, location.impl._offset, 4 * 3, value1, value2, value3);
}

void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4) {
	setFloat4(constantsMemory, location.impl._offset, 4 * 4, value1, value2, value3, value4);
}

void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count) {}

void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, kinc_matrix4x4_t *value) {}

void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, kinc_matrix3x3_t *value) {}

void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, struct kinc_g4_texture *texture, kinc_compute_access_t access) {
	id<MTLTexture> tex = (__bridge id<MTLTexture>)texture->impl._texture.impl._tex;
	[commandEncoder setTexture:tex atIndex:unit.impl._index];
}

void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *texture, kinc_compute_access_t access) {}

void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, struct kinc_g4_texture *texture) {}

void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *target) {}

void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *target) {}

void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_shader(kinc_compute_shader_t *shader) {
	id<MTLComputePipelineState> pipeline = (__bridge id<MTLComputePipelineState>)shader->impl._pipeline;
	[commandEncoder setComputePipelineState:pipeline];
}

void kinc_compute(int x, int y, int z) {
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
