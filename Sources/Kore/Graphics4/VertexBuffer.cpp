#include "Graphics.h"

using namespace Kore;
using namespace Kore::Graphics4;

void Kore_Internal_ConvertVertexStructure(kinc_g4_vertex_structure_t *target, const VertexStructure *source) {
	for (int i = 0; i < source->size; ++i) {
		kinc_g4_vertex_structure_add(target, source->elements[i].name, (kinc_g4_vertex_data_t)source->elements[i].data);
	}
	target->instanced = source->instanced;
}

VertexBuffer::VertexBuffer(int count, const VertexStructure &structure, Usage usage, int instanceDataStepRate) {
	kinc_g4_vertex_structure_t kincStructure;
	kinc_g4_vertex_structure_init(&kincStructure);
	Kore_Internal_ConvertVertexStructure(&kincStructure, &structure);
	kinc_g4_vertex_buffer_init(&kincBuffer, count, &kincStructure, (kinc_g4_usage_t)usage, instanceDataStepRate);
}

VertexBuffer::~VertexBuffer() {
	kinc_g4_vertex_buffer_destroy(&kincBuffer);
}

float *VertexBuffer::lock() {
	return kinc_g4_vertex_buffer_lock_all(&kincBuffer);
}

float *VertexBuffer::lock(int start, int count) {
	return kinc_g4_vertex_buffer_lock(&kincBuffer, start, count);
}

void VertexBuffer::unlock() {
	kinc_g4_vertex_buffer_unlock_all(&kincBuffer);
}

void VertexBuffer::unlock(int count) {
	kinc_g4_vertex_buffer_unlock(&kincBuffer, count);
}

int VertexBuffer::count() {
	return kinc_g4_vertex_buffer_count(&kincBuffer);
}

int VertexBuffer::stride() {
	return kinc_g4_vertex_buffer_stride(&kincBuffer);
}

int VertexBuffer::_set(int offset) {
	return kinc_internal_g4_vertex_buffer_set(&kincBuffer, offset);
}
