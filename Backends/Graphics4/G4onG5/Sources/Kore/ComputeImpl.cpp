#include "pch.h"

#include "ComputeImpl.h"

#include <Kore/Compute/Compute.h>

using namespace Kore;

ComputeShaderImpl::ComputeShaderImpl() {}

ComputeShader::ComputeShader(void* _data, int length) {
	
}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	return unit;
}

void Compute::setFloat(ComputeConstantLocation location, float value) {}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {
	
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target, Access access) {

}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::Texture* texture) {

}

void Compute::setSampledTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {

}

void Compute::setSampledDepthTexture(ComputeTextureUnit unit, Graphics4::RenderTarget* target) {

}

void Compute::setTextureAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {

}

void Compute::setTexture3DAddressing(ComputeTextureUnit unit, Graphics4::TexDir dir, Graphics4::TextureAddressing addressing) {

}

void Compute::setTextureMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {

}

void Compute::setTexture3DMagnificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {

}

void Compute::setTextureMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {

}

void Compute::setTexture3DMinificationFilter(ComputeTextureUnit unit, Graphics4::TextureFilter filter) {

}

void Compute::setTextureMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {

}

void Compute::setTexture3DMipmapFilter(ComputeTextureUnit unit, Graphics4::MipmapFilter filter) {

}

void Compute::setShader(ComputeShader* shader) {
	
}

void Compute::compute(int x, int y, int z) {
	
}
