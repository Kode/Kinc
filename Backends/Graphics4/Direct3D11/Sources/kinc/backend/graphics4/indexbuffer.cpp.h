#include <kinc/graphics4/indexBuffer.h>

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format, kinc_g4_usage_t usage) {
	buffer->impl.count = count;

	if (usage == KINC_G4_USAGE_DYNAMIC) {
		buffer->impl.indices = NULL;
	}
	else {
		buffer->impl.indices = new int[count];
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(unsigned int) * count;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

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

	kinc_microsoft_affirm(device->CreateBuffer(&bufferDesc, nullptr, &buffer->impl.ib));
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	buffer->impl.ib->Release();
	delete[] buffer->impl.indices;
	buffer->impl.indices = NULL;
}

int *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer) {
	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		context->Map(buffer->impl.ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		int *data = (int *)mappedResource.pData;
		return data;
	}
	else {
		return buffer->impl.indices;
	}
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer) {
	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		context->Unmap(buffer->impl.ib, 0);
	}
	else {
		context->UpdateSubresource(buffer->impl.ib, 0, nullptr, buffer->impl.indices, 0, 0);
	}
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.count;
}
