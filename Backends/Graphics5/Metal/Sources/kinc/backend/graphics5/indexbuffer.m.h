#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>

#import <Metal/Metal.h>

id getMetalDevice(void);

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, kinc_g5_index_buffer_format_t format, bool gpuMemory) {
	buffer->impl.count = indexCount;
	buffer->impl.gpu_memory = gpuMemory;
	buffer->impl.format = format;

	id<MTLDevice> device = getMetalDevice();
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
	buffer->impl.metal_buffer = (__bridge_retained void *)[device
	    newBufferWithLength:(format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * indexCount : sizeof(uint32_t) * indexCount)
	                options:options];
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl.metal_buffer;
	buf = nil;
	buffer->impl.metal_buffer = NULL;
}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.metal_buffer;
	return (int *)[buffer contents];
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buf) {
#ifndef KINC_APPLE_SOC
	if (buf->impl.gpu_memory) {
		id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.metal_buffer;
		NSRange range;
		range.location = 0;
		range.length = buf->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * kinc_g5_index_buffer_count(buf)
		                                                                     : sizeof(uint32_t) * kinc_g5_index_buffer_count(buf);
		[buffer didModifyRange:range];
	}
#endif
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
