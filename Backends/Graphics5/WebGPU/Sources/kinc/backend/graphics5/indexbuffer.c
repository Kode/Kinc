#include <kinc/graphics5/indexbuffer.h>

#include <string.h>
#include <stdlib.h>
#include <kinc/graphics5/indexbuffer.h>

extern WGPUDevice device;

kinc_g5_index_buffer_t *kinc_g5_internal_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, kinc_g5_index_buffer_format_t format, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.format = format;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {

}

int *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = buffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? buffer->impl.count * sizeof(uint16_t) : buffer->impl.count * sizeof(uint32_t);
	bDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, 0, bDesc.size);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {

}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
