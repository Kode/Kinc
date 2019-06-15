#include "pch.h"

#include "Compute.h"

using namespace Kore;

ComputeShader::ComputeShader(void *source, int length) {
	kinc_compute_shader_init(&kincImpl, source, length);
}

ComputeShader::~ComputeShader() {
	kinc_compute_shader_destroy(&kincImpl);
}

ComputeConstantLocation ComputeShader::getConstantLocation(const char *name) {
	ComputeConstantLocation location;
	location.kincImpl = kinc_compute_shader_get_constant_location(&kincImpl, name);
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	unit.kincImpl = kinc_compute_shader_get_texture_unit(&kincImpl, name);
	return unit;
}

#ifdef KORE_OPENGL
ShaderStorageBuffer::ShaderStorageBuffer(int count, Graphics4::VertexData type) {
	kinc_shader_storage_buffer_init(&kincImpl, count, (kinc_g4_vertex_data_t)type);
}

ShaderStorageBuffer::~ShaderStorageBuffer() {
	kinc_shader_storage_buffer_destroy(&kincImpl);
}

int *ShaderStorageBuffer::lock() {
	return kinc_shader_storage_buffer_lock(&kincImpl);
}

void ShaderStorageBuffer::unlock() {
	kinc_shader_storage_buffer_unlock(&kincImpl);
}

int ShaderStorageBuffer::count() {
	kinc_shader_storage_buffer_count(&kincImpl);
}

void ShaderStorageBuffer::_set() {
	kinc_shader_storage_buffer_internal_set(&kincImpl);
}
#endif

void Compute::setBool(ComputeConstantLocation location, bool value) {
	kinc_compute_set_bool(location.kincImpl, value);
}

void Compute::setInt(ComputeConstantLocation location, int value) {
	kinc_compute_set_int(location.kincImpl, value);
}

void Compute::setFloat(ComputeConstantLocation location, float value) {
	kinc_compute_set_float(location.kincImpl, value);
}

void Compute::setFloat2(ComputeConstantLocation location, float value1, float value2) {
	kinc_compute_set_float2(location.kincImpl, value1, value2);
}

void Compute::setFloat3(ComputeConstantLocation location, float value1, float value2, float value3) {
	kinc_compute_set_float3(location.kincImpl, value1, value2, value3);
}

void Compute::setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4) {
	kinc_compute_set_float4(location.kincImpl, value1, value2, value3, value4);
}

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {
	kinc_compute_set_floats(location.kincImpl, values, count);
}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {
	kinc_matrix4x4_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 4 * 4);
	kinc_compute_set_matrix4(location.kincImpl, &matrix);
}

void Compute::setMatrix(ComputeConstantLocation location, const mat3 &value) {
	kinc_matrix3x3_t matrix;
	memcpy(&matrix.m, value.data, sizeof(float) * 3 * 3);
	kinc_compute_set_matrix3(location.kincImpl, &matrix);
}

#ifdef KORE_OPENGL
void setBuffer(ShaderStorageBuffer* buffer, int index) {
	kinc_compute_set_buffer(&buffer->kincImpl, index);
}
#endif

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Compute::Access access) {
	kinc_compute_set_texture(unit.kincImpl, &texture->kincTexture, (kinc_compute_access_t)access);
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* texture, Compute::Access access) {
	kinc_compute_set_render_target(unit.kincImpl, &texture->kincRenderTarget, (kinc_compute_access_t)access);
}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture) {
	kinc_compute_set_sampled_texture(unit.kincImpl, &texture->kincTexture);
}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {
	kinc_compute_set_sampled_render_target(unit.kincImpl, &target->kincRenderTarget);
}

void Compute::setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {
	kinc_compute_set_sampled_depth_from_render_target(unit.kincImpl, &target->kincRenderTarget);
}

void Compute::setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
	kinc_compute_set_texture_addressing(unit.kincImpl, (kinc_g4_texture_direction_t)dir, (kinc_g4_texture_addressing_t)addressing);
}

void Compute::setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
	kinc_compute_set_texture_magnification_filter(unit.kincImpl, (kinc_g4_texture_filter_t)filter);
}

void Compute::setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
	kinc_compute_set_texture_minification_filter(unit.kincImpl, (kinc_g4_texture_filter_t)filter);
}

void Compute::setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {
	kinc_compute_set_texture_mipmap_filter(unit.kincImpl, (kinc_g4_mipmap_filter_t)filter);
}

void Compute::setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {
	kinc_compute_set_texture3d_addressing(unit.kincImpl, (kinc_g4_texture_direction_t)dir, (kinc_g4_texture_addressing_t)addressing);
}

void Compute::setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
	kinc_compute_set_texture3d_magnification_filter(unit.kincImpl, (kinc_g4_texture_filter_t)filter);
}

void Compute::setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {
	kinc_compute_set_texture3d_minification_filter(unit.kincImpl, (kinc_g4_texture_filter_t)filter);
}

void Compute::setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {
	kinc_compute_set_texture3d_mipmap_filter(unit.kincImpl, (kinc_g4_mipmap_filter_t)filter);
}

void Compute::setShader(ComputeShader* shader) {
	kinc_compute_set_shader(&shader->kincImpl);
}

void Compute::compute(int x, int y, int z) {
	kinc_compute(x, y, z);
}
