#include "pipeline_functions.h"
#include "pipeline_structs.h"

#include <kinc/log.h>

void kope_metal_render_pipeline_init(kope_metal_device *device, kope_metal_render_pipeline *pipe, const kope_metal_render_pipeline_parameters *parameters) {
	id vertex_function = [getMetalLibrary() newFunctionWithName:[NSString stringWithCString:parameters->vertex.shader.function_name
	                                                                               encoding:NSUTF8StringEncoding]];
	id fragment_function = [getMetalLibrary() newFunctionWithName:[NSString stringWithCString:parameters->fragment.shader.function_name
	                                                                                 encoding:NSUTF8StringEncoding]];

	MTLRenderPipelineDescriptor *render_pipeline_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
	render_pipeline_descriptor.vertexFunction = vertex_function;
	render_pipeline_descriptor.fragmentFunction = fragment_function;

	// TODO

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
