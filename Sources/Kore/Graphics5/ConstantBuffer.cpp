#include "ConstantBuffer.h"

#include <kinc/graphics5/constantbuffer.h>

#include <Kore/Math/Matrix.h>

#include <string.h>

using namespace Kore::Graphics5;

ConstantBuffer::ConstantBuffer(int size) {
	kinc_g5_constant_buffer_init(&kincBuffer, size);
}

ConstantBuffer::~ConstantBuffer() {
	kinc_g5_constant_buffer_destroy(&kincBuffer);
}

void ConstantBuffer::lock() {
	kinc_g5_constant_buffer_lock_all(&kincBuffer);
}

void ConstantBuffer::lock(int start, int count) {
	kinc_g5_constant_buffer_lock(&kincBuffer, start, count);
}

void ConstantBuffer::unlock() {
	kinc_g5_constant_buffer_unlock(&kincBuffer);
}

int ConstantBuffer::size() {
	return kinc_g5_constant_buffer_size(&kincBuffer);
}

void ConstantBuffer::setBool(int offset, bool value) {
	kinc_g5_constant_buffer_set_bool(&kincBuffer, offset, value);
}

void ConstantBuffer::setInt(int offset, int value) {
	kinc_g5_constant_buffer_set_int(&kincBuffer, offset, value);
}

void ConstantBuffer::setFloat(int offset, float value) {
	kinc_g5_constant_buffer_set_float(&kincBuffer, offset, value);
}

void ConstantBuffer::setFloat2(int offset, float value1, float value2) {
	kinc_g5_constant_buffer_set_float2(&kincBuffer, offset, value1, value2);
}

void ConstantBuffer::setFloat2(int offset, vec2 value) {
	kinc_g5_constant_buffer_set_float2(&kincBuffer, offset, value.x(), value.y());
}

void ConstantBuffer::setFloat3(int offset, float value1, float value2, float value3) {
	kinc_g5_constant_buffer_set_float3(&kincBuffer, offset, value1, value2, value3);
}

void ConstantBuffer::setFloat3(int offset, vec3 value) {
	kinc_g5_constant_buffer_set_float3(&kincBuffer, offset, value.x(), value.y(), value.z());
}

void ConstantBuffer::setFloat4(int offset, float value1, float value2, float value3, float value4) {
	kinc_g5_constant_buffer_set_float4(&kincBuffer, offset, value1, value2, value3, value4);
}

void ConstantBuffer::setFloat4(int offset, vec4 value) {
	kinc_g5_constant_buffer_set_float4(&kincBuffer, offset, value.x(), value.y(), value.z(), value.w());
}

void ConstantBuffer::setFloats(int offset, float *values, int count) {
	kinc_g5_constant_buffer_set_floats(&kincBuffer, offset, values, count);
}

void ConstantBuffer::setMatrix(int offset, const mat3 &value) {
	kinc_matrix3x3_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 3 * 3);
	kinc_g5_constant_buffer_set_matrix3(&kincBuffer, offset, &matrix);
}

void ConstantBuffer::setMatrix(int offset, const mat4 &value) {
	kinc_matrix4x4_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 4 * 4);
	kinc_g5_constant_buffer_set_matrix4(&kincBuffer, offset, &matrix);
}
