#include "device_functions.h"

#include "metalunit.h"

#include <kope/graphics5/device.h>
#include <kope/util/align.h>

#include <kinc/log.h>
#include <kinc/window.h>

#include <assert.h>

void kope_metal_device_create(kope_g5_device *device, const kope_g5_device_wishlist *wishlist) {
	id<MTLDevice> metal_device = MTLCreateSystemDefaultDevice();
	getMetalLayer().device = metal_device;
	device->metal.device = (__bridge_retained void *)metal_device;
	device->metal.library = (__bridge_retained void *)[metal_device newDefaultLibrary];
}

void kope_metal_device_destroy(kope_g5_device *device) {}

void kope_metal_device_set_name(kope_g5_device *device, const char *name) {}

void kope_metal_device_create_buffer(kope_g5_device *device, const kope_g5_buffer_parameters *parameters, kope_g5_buffer *buffer) {
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
#ifdef KINC_APPLE_SOC
	options |= MTLResourceStorageModeShared;
#else
	if (gpuMemory) {
		options |= MTLResourceStorageModeManaged;
	}
	else {
		options |= MTLResourceStorageModeShared;
	}
#endif
	id<MTLBuffer> metal_buffer = [metal_device newBufferWithLength:parameters->size options:options];
	buffer->metal.buffer = (__bridge_retained void *)metal_buffer;
}

void kope_metal_device_create_command_list(kope_g5_device *device, kope_g5_command_list_type type, kope_g5_command_list *list) {
	id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;
	id<MTLCommandQueue> command_queue = [metal_device newCommandQueue];
	list->metal.command_queue = (__bridge_retained void *)command_queue;
	list->metal.command_buffer = (__bridge_retained void *)[command_queue commandBuffer];
}

void kope_metal_device_create_texture(kope_g5_device *device, const kope_g5_texture_parameters *parameters, kope_g5_texture *texture) {}

static kope_g5_texture framebuffer;

kope_g5_texture *kope_metal_device_get_framebuffer(kope_g5_device *device) {
	CAMetalLayer *metal_layer = getMetalLayer();
	id<CAMetalDrawable> drawable = [metal_layer nextDrawable];
	framebuffer.metal.texture = (__bridge_retained void *)drawable.texture;
	return &framebuffer;
}

kope_g5_texture_format kope_metal_device_framebuffer_format(kope_g5_device *device) {
	return KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM;
}

void kope_metal_device_execute_command_list(kope_g5_device *device, kope_g5_command_list *list) {
	id<MTLCommandBuffer> command_buffer = (__bridge id<MTLCommandBuffer>)list->metal.command_buffer;
	[command_buffer commit];

	id<MTLCommandQueue> command_queue = (__bridge id<MTLCommandQueue>)list->metal.command_queue;
	command_buffer = [command_queue commandBuffer];
	list->metal.command_buffer = (__bridge_retained void *)[command_queue commandBuffer];
}

void kope_metal_device_wait_until_idle(kope_g5_device *device) {}

void kope_metal_device_create_descriptor_set(kope_g5_device *device, uint32_t descriptor_count, uint32_t dynamic_descriptor_count,
                                             uint32_t bindless_descriptor_count, uint32_t sampler_count, kope_metal_descriptor_set *set) {}

void kope_metal_device_create_sampler(kope_g5_device *device, const kope_g5_sampler_parameters *parameters, kope_g5_sampler *sampler) {}

void kope_metal_device_create_raytracing_volume(kope_g5_device *device, kope_g5_buffer *vertex_buffer, uint64_t vertex_count, kope_g5_buffer *index_buffer,
                                                uint32_t index_count, kope_g5_raytracing_volume *volume) {}

void kope_metal_device_create_raytracing_hierarchy(kope_g5_device *device, kope_g5_raytracing_volume **volumes, kinc_matrix4x4_t *volume_transforms,
                                                   uint32_t volumes_count, kope_g5_raytracing_hierarchy *hierarchy) {}

void kope_metal_device_create_query_set(kope_g5_device *device, const kope_g5_query_set_parameters *parameters, kope_g5_query_set *query_set) {}

uint32_t kope_metal_device_align_texture_row_bytes(kope_g5_device *device, uint32_t row_bytes) {
	return 0;
}

void kope_metal_device_create_fence(kope_g5_device *device, kope_g5_fence *fence) {}

void kope_metal_device_signal(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value) {}

void kope_metal_device_wait(kope_g5_device *device, kope_g5_command_list_type list_type, kope_g5_fence *fence, uint64_t value) {}
