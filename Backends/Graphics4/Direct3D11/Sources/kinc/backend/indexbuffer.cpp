#include "pch.h"

#include "Direct3D11.h"

#include <kinc/graphics4/indexBuffer.h>

#include <Kore/SystemMicrosoft.h>

#include <Windows.h>
#include <d3d11.h>

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format) {
	buffer->impl.count = count;
	buffer->impl.indices = new int[count];

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(unsigned int) * count;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	kinc_microsoft_affirm(device->CreateBuffer(&bufferDesc, nullptr, &buffer->impl.ib));
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	buffer->impl.ib->Release();
	delete[] buffer->impl.indices;
	buffer->impl.indices = NULL;
}

int *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.indices;
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer) {
	context->UpdateSubresource(buffer->impl.ib, 0, nullptr, buffer->impl.indices, 0, 0);
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.count;
}
