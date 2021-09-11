#pragma once

typedef struct {
	// ShaderStorageBufferImpl(int count, Graphics4::VertexData type);
	// void unset();
	int *data;
	int myCount;
	int myStride;
	unsigned bufferId;
	// static ShaderStorageBuffer* current;
} kinc_compute_shader_storage_buffer_impl_t;
