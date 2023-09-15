#ifdef KINC_KONG

#include <kinc/graphics4/constantbuffer.h>

void kinc_g4_constant_buffer_init(kinc_g4_constant_buffer *buffer, size_t size) {
	buffer->impl.size = size;
	buffer->impl.last_start = 0;
	buffer->impl.last_size = size;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (UINT)get_multiple_of_16(size);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &buffer->impl.buffer));
}

void kinc_g4_constant_buffer_destroy(kinc_g4_constant_buffer *buffer) {
	buffer->impl.buffer->lpVtbl->Release(buffer->impl.buffer);
}

uint8_t *kinc_g4_constant_buffer_lock_all(kinc_g4_constant_buffer *buffer) {
	return kinc_g4_constant_buffer_lock(buffer, 0, kinc_g4_constant_buffer_size(buffer));
}

uint8_t *kinc_g4_constant_buffer_lock(kinc_g4_constant_buffer *buffer, size_t start, size_t size) {
	buffer->impl.last_start = start;
	buffer->impl.last_size = size;

	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	memset(&mapped_resource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
	dx_ctx.context->lpVtbl->Map(dx_ctx.context, (ID3D11Resource *)buffer->impl.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	uint8_t *data = (uint8_t *)mapped_resource.pData;
	return &data[start];
}

void kinc_g4_constant_buffer_unlock_all(kinc_g4_constant_buffer *buffer) {
	kinc_g4_constant_buffer_unlock(buffer, buffer->impl.last_size);

}

void kinc_g4_constant_buffer_unlock(kinc_g4_constant_buffer *buffer, size_t count) {
	dx_ctx.context->lpVtbl->Unmap(dx_ctx.context, (ID3D11Resource *)buffer->impl.buffer, 0);
}

size_t kinc_g4_constant_buffer_size(kinc_g4_constant_buffer *buffer) {
	return buffer->impl.size;
}

#endif
