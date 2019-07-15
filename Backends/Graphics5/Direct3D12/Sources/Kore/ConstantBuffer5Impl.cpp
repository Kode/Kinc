#include "pch.h"

#include <Kinc/Graphics5/ConstantBuffer.h>

#include "Direct3D12.h"
#include <Kore/SystemMicrosoft.h>

bool kinc_g5_transposeMat3 = true;
bool kinc_g5_transposeMat4 = true;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = nullptr;

	kinc_microsoft_affirm(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
	                                                  &CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	                                                      IID_GRAPHICS_PPV_ARGS(&buffer->impl._buffer)));

	void* p;
	buffer->impl._buffer->Map(0, nullptr, &p);
	ZeroMemory(p, size);
	buffer->impl._buffer->Unmap(0, nullptr);
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	D3D12_RANGE range;
	range.Begin = start;
	range.End = range.Begin + count;
	uint8_t *p;
	buffer->impl._buffer->Map(0, &range, (void **)&p);
	buffer->data = &p[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart;
	range.End = range.Begin + buffer->impl.lastCount;
	buffer->impl._buffer->Unmap(0, &range);
	buffer->data = nullptr;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}
