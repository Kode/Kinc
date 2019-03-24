#include "pch.h"

#include "Direct3D11.h"

#include <Kinc/Graphics4/VertexBuffer.h>

#include <Kore/SystemMicrosoft.h>

void Kinc_G4_VertexBuffer_Create(Kinc_G4_VertexBuffer *buffer, int count, Kinc_G4_VertexStructure *structure, Kinc_G4_Usage usage, int instance_data_step_rate) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		switch (structure->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			buffer->impl.stride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			buffer->impl.stride += 2 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			buffer->impl.stride += 3 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			buffer->impl.stride += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_COLOR:
			buffer->impl.stride += 1 * 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			buffer->impl.stride += 4 * 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_SHORT2_NORM:
			buffer->impl.stride += 2 * 2;
			break;
		case KINC_G4_VERTEX_DATA_SHORT4_NORM:
			buffer->impl.stride += 4 * 2;
			break;
		}
	}

	buffer->impl.vertices = new float[buffer->impl.stride / 4 * count];
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.CPUAccessFlags = 0;
	
	buffer->impl.usage = usage;
	switch (usage) {
	case KINC_G4_USAGE_STATIC:
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	case KINC_G4_USAGE_DYNAMIC:
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	case KINC_G4_USAGE_READABLE:
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}
	
	bufferDesc.ByteWidth = buffer->impl.stride * count;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	Kinc_Microsoft_Affirm(device->CreateBuffer(&bufferDesc, nullptr, &buffer->impl.vb));
}

void Kinc_G4_VertexBuffer_Destroy(Kinc_G4_VertexBuffer *buffer) {
	buffer->impl.vb->Release();
	delete[] buffer->impl.vertices;
}

float *Kinc_G4_VertexBuffer_LockAll(Kinc_G4_VertexBuffer *buffer) {
	return Kinc_G4_VertexBuffer_Lock(buffer, 0, buffer->impl.count);
}

float *Kinc_G4_VertexBuffer_Lock(Kinc_G4_VertexBuffer *buffer, int start, int count) {
	buffer->impl.lockStart = start;
	buffer->impl.lockCount = count;
	return &buffer->impl.vertices[start * buffer->impl.stride / 4];
}

void Kinc_G4_VertexBuffer_UnlockAll(Kinc_G4_VertexBuffer *buffer) {
	Kinc_G4_VertexBuffer_Unlock(buffer, buffer->impl.lockCount);
}

void Kinc_G4_VertexBuffer_Unlock(Kinc_G4_VertexBuffer *buffer, int count) {
	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		context->Map(buffer->impl.vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &buffer->impl.vertices[buffer->impl.lockStart * buffer->impl.stride / 4], (count * buffer->impl.stride / 4) * sizeof(float));
		context->Unmap(buffer->impl.vb, 0);
	}
	else {
		context->UpdateSubresource(buffer->impl.vb, 0, nullptr, buffer->impl.vertices, 0, 0);
	}
}

int Kinc_Internal_G4_VertexBuffer_Set(Kinc_G4_VertexBuffer *buffer, int offset) {
	// UINT stride = myStride;
	// UINT internaloffset = 0;
	// context->IASetVertexBuffers(0, 1, &vb, &stride, &internaloffset);
	return 0;
}

int Kinc_G4_VertexBuffer_Count(Kinc_G4_VertexBuffer* buffer) {
	return buffer->impl.count;
}

int Kinc_G4_VertexBuffer_Stride(Kinc_G4_VertexBuffer* buffer) {
	return buffer->impl.stride;
}
