#include "pch.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <Kore/SystemMicrosoft.h>

#include "Direct3D9.h"

struct kinc_g4_vertex_buffer *kinc_internal_current_vertex_buffer = NULL;

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                int instance_data_step_rate) {
	buffer->impl.myCount = count;
	buffer->impl.instanceDataStepRate = instance_data_step_rate;
	DWORD usageFlags = D3DUSAGE_WRITEONLY;
	if (usage == Kore::Graphics4::DynamicUsage) {
		usageFlags = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
	}

	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		switch (structure->elements[i].data) {
		case KINC_G4_VERTEX_DATA_FLOAT1:
			buffer->impl.myStride += 4 * 1;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT2:
			buffer->impl.myStride += 4 * 2;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT3:
			buffer->impl.myStride += 4 * 3;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4:
			buffer->impl.myStride += 4 * 4;
			break;
		case KINC_G4_VERTEX_DATA_COLOR:
			buffer->impl.myStride += 4;
			break;
		case KINC_G4_VERTEX_DATA_FLOAT4X4:
			buffer->impl.myStride += 4 * 4 * 4;
			break;
		}
	}

	kinc_microsoft_affirm(device->CreateVertexBuffer(kinc_g4_vertex_buffer_stride(buffer) * count, usageFlags, 0, D3DPOOL_DEFAULT, &buffer->impl.vb, 0));
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	buffer->impl.vb->Release();
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g4_vertex_buffer_lock(buffer, 0, kinc_g4_vertex_buffer_count(buffer));
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	float *vertices;
	kinc_internal_vertex_buffer_unset(buffer);
	kinc_microsoft_affirm(buffer->impl.vb->Lock(start, count * kinc_g4_vertex_buffer_stride(buffer), (void **)&vertices, D3DLOCK_DISCARD));
	return vertices;
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_microsoft_affirm(buffer->impl.vb->Unlock());
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	kinc_microsoft_affirm(buffer->impl.vb->Unlock());
}

int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset) {
	buffer->impl._offset = offset;
	if (buffer->impl.instanceDataStepRate == 0) {
		kinc_internal_current_vertex_buffer = buffer;
	}
	else {
		kinc_microsoft_affirm(device->SetStreamSourceFreq(offset, (D3DSTREAMSOURCE_INSTANCEDATA | buffer->impl.instanceDataStepRate)));
	}
	kinc_microsoft_affirm(device->SetStreamSource(offset, buffer->impl.vb, 0, kinc_g4_vertex_buffer_stride(buffer)));
	return 0;
}

void kinc_internal_vertex_buffer_unset(struct kinc_g4_vertex_buffer *buffer) {
	if (kinc_internal_current_vertex_buffer == buffer) {
		kinc_microsoft_affirm(device->SetStreamSource(0, NULL, 0, 0));
		kinc_internal_current_vertex_buffer = NULL;
	}
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}
