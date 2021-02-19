#include "pch.h"

#include "ogl.h"

#include <kinc/compute/compute.h>

#if defined(KORE_WINDOWS) || (defined(KORE_LINUX) && defined(GL_VERSION_4_3)) || (defined(KORE_ANDROID) && defined(GL_ES_VERSION_3_1))
#define HAS_COMPUTE
#endif

kinc_shader_storage_buffer_t *currentStorageBuffer = NULL;

static void unset(kinc_shader_storage_buffer_t *buffer) {
	if (currentStorageBuffer == buffer) currentStorageBuffer = NULL;
}

void kinc_shader_storage_buffer_init(kinc_shader_storage_buffer_t *buffer, int indexCount, kinc_g4_vertex_data_t type) {
	buffer->impl.myCount = indexCount;
	buffer->impl.myStride = 0;
	switch (type) {
	case KINC_G4_VERTEX_DATA_COLOR:
		buffer->impl.myStride += 1 * 4;
		break;
	case KINC_G4_VERTEX_DATA_FLOAT1:
		buffer->impl.myStride += 1 * 4;
		break;
	case KINC_G4_VERTEX_DATA_FLOAT2:
		buffer->impl.myStride += 2 * 4;
		break;
	case KINC_G4_VERTEX_DATA_FLOAT3:
		buffer->impl.myStride += 3 * 4;
		break;
	case KINC_G4_VERTEX_DATA_FLOAT4:
		buffer->impl.myStride += 4 * 4;
		break;
	case KINC_G4_VERTEX_DATA_FLOAT4X4:
		buffer->impl.myStride += 4 * 4 * 4;
		break;
	case KINC_G4_VERTEX_DATA_SHORT2_NORM:
		buffer->impl.myStride += 2 * 2;
		break;
	case KINC_G4_VERTEX_DATA_SHORT4_NORM:
		buffer->impl.myStride += 2 * 4;
		break;
	case KINC_G4_VERTEX_DATA_NONE:
		break;
	}
#ifdef HAS_COMPUTE
	glGenBuffers(1, &buffer->impl.bufferId);
	glCheckErrors();
#endif
	buffer->impl.data = (int*)malloc(sizeof(int) * indexCount);
}

void kinc_shader_storage_buffer_destroy(kinc_shader_storage_buffer_t *buffer) {
	unset(buffer);
	free(buffer->impl.data);
}

int *kinc_shader_storage_buffer_lock(kinc_shader_storage_buffer_t *buffer) {
	return buffer->impl.data;
}

void kinc_shader_storage_buffer_unlock(kinc_shader_storage_buffer_t *buffer) {
#ifdef HAS_COMPUTE
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
	glBufferData(GL_SHADER_STORAGE_BUFFER, buffer->impl.myCount * buffer->impl.myStride, buffer->impl.data, GL_STATIC_DRAW);
	glCheckErrors();
#endif
}

void kinc_shader_storage_buffer_internal_set(kinc_shader_storage_buffer_t *buffer) {
	currentStorageBuffer = buffer;
#ifdef HAS_COMPUTE
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->impl.bufferId);
	glCheckErrors();
#endif
}

int kinc_shader_storage_buffer_count(kinc_shader_storage_buffer_t *buffer) {
	return buffer->impl.myCount;
}
