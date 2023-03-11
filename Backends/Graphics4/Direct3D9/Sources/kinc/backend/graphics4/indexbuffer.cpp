#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>

#include <kinc/backend/SystemMicrosoft.h>

#include "Direct3D9.h"

struct kinc_g4_index_buffer *kinc_internal_current_index_buffer = NULL;

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format, kinc_g4_usage_t usage) {
	buffer->impl.count = count;
	buffer->impl.format = format;

	DWORD usageFlags = D3DUSAGE_WRITEONLY;
	if (usage == KINC_G4_USAGE_DYNAMIC) {
		usageFlags = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	}

	kinc_microsoft_affirm(device->CreateIndexBuffer(
	    sizeof(int) * count, usageFlags, format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_DEFAULT, &buffer->impl.ib, 0));
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	buffer->impl.ib->Release();
}

static int kinc_g4_internal_index_buffer_stride(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.format == KINC_G4_INDEX_BUFFER_FORMAT_32BIT ? 4 : 2;
}

void *kinc_g4_index_buffer_lock_all(kinc_g4_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_lock(buffer, 0, kinc_g4_index_buffer_count(buffer));
}

void *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer, int start, int count) {
	uint8_t *data;
	kinc_microsoft_affirm(buffer->impl.ib->Lock(0, sizeof(int) * kinc_g4_index_buffer_count(buffer), (void **)&data, D3DLOCK_DISCARD));
	return &data[start * kinc_g4_internal_index_buffer_stride(buffer)];
}

void kinc_g4_index_buffer_unlock_all(kinc_g4_index_buffer_t *buffer) {
	kinc_microsoft_affirm(buffer->impl.ib->Unlock());
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer, int count) {
	kinc_g4_index_buffer_unlock_all(buffer);
}

void kinc_internal_g4_index_buffer_set(kinc_g4_index_buffer_t *buffer) {
	kinc_internal_current_index_buffer = buffer;
	kinc_microsoft_affirm(device->SetIndices(buffer->impl.ib));
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.count;
}
