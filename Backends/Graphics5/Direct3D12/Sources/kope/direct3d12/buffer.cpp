#include "buffer_functions.h"

#include <kope/graphics5/buffer.h>

#include <kinc/backend/SystemMicrosoft.h>

static uint64_t find_max_execution_index_all(kope_g5_buffer *buffer) {
	uint64_t max_execution_index = 0;

	for (uint32_t range_index = 0; range_index < buffer->d3d12.ranges_count; ++range_index) {
		uint64_t execution_index = buffer->d3d12.ranges[range_index].execution_index;
		if (execution_index > max_execution_index) {
			max_execution_index = execution_index;
		}
	}

	return max_execution_index;
}

static uint64_t find_max_execution_index(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	uint64_t max_execution_index = 0;

	for (uint32_t range_index = 0; range_index < buffer->d3d12.ranges_count; ++range_index) {
		kope_d3d12_buffer_range range = buffer->d3d12.ranges[range_index];

		if (range.size == UINT64_MAX || (offset >= range.offset && offset < range.offset + range.size) ||
		    (offset + size > range.offset && offset + size <= range.offset + range.size)) {
			uint64_t execution_index = buffer->d3d12.ranges[range_index].execution_index;
			if (execution_index > max_execution_index) {
				max_execution_index = execution_index;
			}
		}
	}

	return max_execution_index;
}

void kope_d3d12_buffer_set_name(kope_g5_buffer *buffer, const char *name) {
	wchar_t wstr[1024];
	kinc_microsoft_convert_string(wstr, name, 1024);
	buffer->d3d12.resource->SetName(wstr);
}

void kope_d3d12_buffer_destroy(kope_g5_buffer *buffer) {
	buffer->d3d12.resource->Release();
}

void *kope_d3d12_buffer_try_to_lock_all(kope_g5_buffer *buffer) {
	if (check_for_fence(buffer->d3d12.device->d3d12.execution_fence, find_max_execution_index_all(buffer))) {
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
	               find_max_execution_index_all(buffer));

	buffer->d3d12.locked_data_offset = 0;
	buffer->d3d12.locked_data_size = UINT64_MAX;

	buffer->d3d12.resource->Map(0, NULL, &buffer->d3d12.locked_data);
	return buffer->d3d12.locked_data;
}

void *kope_d3d12_buffer_try_to_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	if (check_for_fence(buffer->d3d12.device->d3d12.execution_fence, find_max_execution_index(buffer, offset, size))) {
		D3D12_RANGE read_range;
		D3D12_RANGE *read_range_pointer = NULL;

		if (buffer->d3d12.cpu_read) {
			read_range.Begin = offset;
			read_range.End = offset + size;
			read_range_pointer = &read_range;
		}

		buffer->d3d12.locked_data_offset = offset;
		buffer->d3d12.locked_data_size = size;

		uint8_t *data = nullptr;
		buffer->d3d12.resource->Map(0, read_range_pointer, (void **)&data);
		buffer->d3d12.locked_data = data + offset;
		return buffer->d3d12.locked_data;
	}
	else {
		return NULL;
	}
}

void *kope_d3d12_buffer_lock(kope_g5_buffer *buffer, uint64_t offset, uint64_t size) {
	wait_for_fence(buffer->d3d12.device, buffer->d3d12.device->d3d12.execution_fence, buffer->d3d12.device->d3d12.execution_event,
	               find_max_execution_index(buffer, offset, size));
	D3D12_RANGE read_range;
	D3D12_RANGE *read_range_pointer = NULL;

	if (buffer->d3d12.cpu_read) {
		read_range.Begin = offset;
		read_range.End = offset + size;
		read_range_pointer = &read_range;
	}

	buffer->d3d12.locked_data_offset = offset;
	buffer->d3d12.locked_data_size = size;

	uint8_t *data = nullptr;
	buffer->d3d12.resource->Map(0, read_range_pointer, (void **)&data);
	buffer->d3d12.locked_data = data + offset;
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
