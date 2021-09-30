#include <Kinc/Graphics5/ConstantBuffer.h>

#include <kinc/backend/SystemMicrosoft.h>

bool kinc_g5_transposeMat3 = false;
bool kinc_g5_transposeMat4 = false;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	kinc_microsoft_affirm(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      IID_GRAPHICS_PPV_ARGS(&buffer->impl.constant_buffer)));

	void *p;
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, NULL, &p);
	ZeroMemory(p, size);
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, NULL);
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
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, &range, (void **)&p);
	buffer->data = &p[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart;
	range.End = range.Begin + buffer->impl.lastCount;
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, &range);
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}
