#include "pch.h"

#include "ComputeImpl.h"
#include "Direct3D11.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Math/Core.h>
#include <Kore/WinError.h>

using namespace Kore;

ComputeShaderImpl::ComputeShaderImpl() {}

ComputeShader::ComputeShader(void* _data, int length) {
	unsigned index = 0;
	u8* data = (u8*)_data;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		attributes[name] = data[index++];
	}

	u8 texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		textures[name] = data[index++];
	}

	u8 constantCount = data[index++];
	constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		ComputeShaderConstant constant;
		constant.offset = data[index++];
		constant.size = data[index++];
		constants[name] = constant;
		constantsSize = constant.offset + constant.size;
	}

	this->data = &data[index];
	this->length = length - index;

	affirm(device->CreateComputeShader(this->data, this->length, nullptr, (ID3D11ComputeShader**)&shader));
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
	ID3D11ShaderResourceView* nullView = nullptr;
	context->PSSetShaderResources(0, 1, &nullView);

	context->CSSetUnorderedAccessViews(0, 1, &texture->computeView, nullptr);
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
	context->CSSetShader((ID3D11ComputeShader*)shader->shader, nullptr, 0);
}

void Compute::compute(int x, int y, int z) {
	context->Dispatch(x, y, z);

	ID3D11UnorderedAccessView* nullView = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullView, nullptr);
}
