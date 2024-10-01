#include "buffer_functions.h"

#include <kope/graphics5/buffer.h>

#include <kinc/backend/SystemMicrosoft.h>

void kope_d3d12_buffer_set_name(kope_g5_buffer *buffer, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	buffer->d3d12.resource->SetName(wstr);
}

void kope_d3d12_buffer_destroy(kope_g5_buffer *buffer) {
	buffer->d3d12.resource->Release();
}

void *kope_d3d12_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	if (check_for_fence(buffer->d3d12.device->d3d12.execution_fence, buffer->d3d12.latest_execution_index)) {
		buffer->d3d12.locked_data_offset = 0;
		buffer->d3d12.locked_data_size = UINT64_MAX;

		buffer->d3d12.resource->Map(0, NULL, &buffer->d3d12.locked_data);
		return buffer->d3d12.locked_data;
	}
	else {
		return NULL;
	}
}

void *kope_d3d12_buffer_lock_all(kope_g5_buffer *buffer) {
	wait_for_fence(buffer->d3d12.device, buffer->d3d12.device->d3d12.execution_fence, buffer->d3d12.device->d3d12.execution_event,
	               buffer->d3d12.latest_execution_index);

	buffer->d3d12.locked_data_offset = 0;
	buffer->d3d12.locked_data_size = UINT64_MAX;

	buffer->d3d12.resource->Map(0, NULL, &buffer->d3d12.locked_data);
	return buffer->d3d12.locked_data;
}

void *kope_d3d12_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	if (check_for_fence(buffer->d3d12.device->d3d12.execution_fence, buffer->d3d12.latest_execution_index)) {
		D3D12_RANGE read_range;
		D3D12_RANGE *read_range_pointer = NULL;

		if (buffer->d3d12.cpu_read) {
			read_range.Begin = offset;
			read_range.End = offset + size;
			read_range_pointer = &read_range;
		}

		buffer->d3d12.locked_data_offset = offset;
		buffer->d3d12.locked_data_size = size;

		buffer->d3d12.resource->Map(0, read_range_pointer, &buffer->d3d12.locked_data);
		return buffer->d3d12.locked_data;
	}
	else {
		return NULL;
	}
}

void *kope_d3d12_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	wait_for_fence(buffer->d3d12.device, buffer->d3d12.device->d3d12.execution_fence, buffer->d3d12.device->d3d12.execution_event,
	               buffer->d3d12.latest_execution_index);
	D3D12_RANGE read_range;
	D3D12_RANGE *read_range_pointer = NULL;

	if (buffer->d3d12.cpu_read) {
		read_range.Begin = offset;
		read_range.End = offset + size;
		read_range_pointer = &read_range;
	}

	buffer->d3d12.locked_data_offset = offset;
	buffer->d3d12.locked_data_size = size;

	buffer->d3d12.resource->Map(0, read_range_pointer, &buffer->d3d12.locked_data);
	return buffer->d3d12.locked_data;
}

void kope_d3d12_buffer_unlock(kope_g5_buffer *buffer) {
	D3D12_RANGE written;
	D3D12_RANGE *written_pointer = NULL;

	if (buffer->d3d12.cpu_write && buffer->d3d12.locked_data_size < UINT64_MAX) {
		written.Begin = buffer->d3d12.locked_data_offset;
		written.End = buffer->d3d12.locked_data_offset + buffer->d3d12.locked_data_size;
	}

	buffer->d3d12.resource->Unmap(0, written_pointer);
}
