#include "pch.h"

#include "Direct3D11.h"

#include <Kinc/Graphics4/IndexBuffer.h>

#include <Kore/SystemMicrosoft.h>

#include <Windows.h>
#include <d3d11.h>

void Kinc_G4_IndexBuffer_Create(Kinc_G4_IndexBuffer *buffer, int count) {
	buffer->impl.count = count;
	buffer->impl.indices = new int[count];

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(unsigned int) * count;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	Kinc_Microsoft_Affirm(device->CreateBuffer(&bufferDesc, nullptr, &buffer->impl.ib));
}

void Kinc_G4_IndexBuffer_Destroy(Kinc_G4_IndexBuffer *buffer) {
	buffer->impl.ib->Release();
	delete[] buffer->impl.indices;
	buffer->impl.indices = NULL;
}

int *Kinc_G4_IndexBuffer_Lock(Kinc_G4_IndexBuffer *buffer) {
	return buffer->impl.indices;
}

void Kinc_G4_IndexBuffer_Unlock(Kinc_G4_IndexBuffer *buffer) {
	context->UpdateSubresource(buffer->impl.ib, 0, nullptr, buffer->impl.indices, 0, 0);
}

int Kinc_G4_IndexBuffer_Count(Kinc_G4_IndexBuffer *buffer) {
	return buffer->impl.count;
}
