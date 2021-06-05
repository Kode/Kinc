#include <kinc/graphics5/shader.h>
#include <kinc/graphics5/vertexbuffer.h>

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>

#import <Metal/Metal.h>

id getMetalDevice();
id getMetalEncoder();

kinc_g5_vertex_buffer_t *currentVertexBuffer = nullptr;
extern kinc_g5_index_buffer_t *currentIndexBuffer;

static void vertex_buffer_unset(kinc_g5_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer) currentVertexBuffer = nullptr;
}

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory, int instanceDataStepRate) {
	memset(&buffer->impl, 0, sizeof(buffer->impl));
	buffer->impl.myCount = count;
	buffer->impl.gpuMemory = gpuMemory;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		switch (element.data) {
		case KINC_G4_VERTEX_DATA_COLOR:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT1:
			buffer->impl.myStride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			buffer->impl.myStride += 2 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			buffer->impl.myStride += 3 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			buffer->impl.myStride += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			buffer->impl.myStride += 2 * 2;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			buffer->impl.myStride += 4 * 2;
			break;
		case KINC_G4_VERTEX_DATA_NONE:
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			assert(false);
			break;
		}
	}

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLCPUCacheModeWriteCombined;
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
	buffer->impl.mtlBuffer = [device newBufferWithLength:count * buffer->impl.myStride options:options];
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	buffer->impl.mtlBuffer = 0;
	vertex_buffer_unset(buffer);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buf) {
	buf->impl.lastStart = 0;
	buf->impl.lastCount = kinc_g5_vertex_buffer_count(buf);
	id<MTLBuffer> buffer = buf->impl.mtlBuffer;
	float* floats = (float*)[buffer contents];
	return floats;
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buf, int start, int count) {
	buf->impl.lastStart = start;
	buf->impl.lastCount = count;
	id<MTLBuffer> buffer = buf->impl.mtlBuffer;
	float* floats = (float*)[buffer contents];
	return &floats[start * buf->impl.myStride / sizeof(float)];
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buf) {
#ifndef KINC_APPLE_SOC
	if (buf->impl.gpuMemory) {
		id<MTLBuffer> buffer = buf->impl.mtlBuffer;
		NSRange range;
		range.location = buf->impl.lastStart * buf->impl.myStride;
		range.length = buf->impl.lastCount * buf->impl.myStride;
		[buffer didModifyRange:range];
	}
#endif
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buf, int count) {
#ifndef KINC_APPLE_SOC
	if (buf->impl.gpuMemory) {
		id<MTLBuffer> buffer = buf->impl.mtlBuffer;
		NSRange range;
		range.location = buf->impl.lastStart * buf->impl.myStride;
		range.length = count * buf->impl.myStride;
		[buffer didModifyRange:range];
	}
#endif
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer, int offset_) {
	currentVertexBuffer = buffer;
	if (currentIndexBuffer != nullptr) kinc_g5_internal_index_buffer_set(currentIndexBuffer);

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setVertexBuffer:buffer->impl.mtlBuffer offset:offset_ * buffer->impl.myStride atIndex:0];

	return offset_;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}
