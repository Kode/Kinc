#include "pch.h"

#include "ComputeImpl.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Math/Core.h>

using namespace Kore;

ComputeShaderImpl::ComputeShaderImpl(void* source, int length) {}

ComputeShader::ComputeShader(void* _data, int length) : ComputeShaderImpl(_data, length) {}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	return unit;
}

void Compute::setBool(ComputeConstantLocation location, bool value) {}

void Compute::setInt(ComputeConstantLocation location, int value) {}

void Compute::setFloat(ComputeConstantLocation location, float value) {}

void Compute::setFloat2(ComputeConstantLocation location, float value1, float value2) {}

void Compute::setFloat3(ComputeConstantLocation location, float value1, float value2, float value3) {}

void Compute::setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4) {}

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat3& value) {}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target, Access access) {}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture) {}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {}

void Compute::setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {}

void Compute::setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {}

void Compute::setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {}

void Compute::setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {}

void Compute::setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {}

void Compute::setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {}

void Compute::setShader(ComputeShader* shader) {}

void Compute::compute(int x, int y, int z) {}
