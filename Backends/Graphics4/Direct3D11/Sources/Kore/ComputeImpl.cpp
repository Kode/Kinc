#include "pch.h"

#include "ComputeImpl.h"
#include "Direct3D11.h"

#include <Kore/Compute/Compute.h>
#include <Kore/Math/Core.h>
#include <Kore/SystemMicrosoft.h>

using namespace Kore;

namespace {
	u8 constantsMemory[1024 * 4];

	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	void setFloat(u8* constants, u32 offset, u32 size, float value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, u32 offset, u32 size, float value1, float value2) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, u32 offset, u32 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, u32 offset, u32 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}
}

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
		constant.offset = data[index];
		index += 4;
		constant.size = data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;
		constants[name] = constant;
		constantsSize = constant.offset + constant.size;
	}

	this->data = &data[index];
	this->length = length - index;

	Microsoft::affirm(device->CreateComputeShader(this->data, this->length, nullptr, (ID3D11ComputeShader**)&shader));

	Microsoft::affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &constantBuffer));
}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	if (constants.find(name) != constants.end()) {
		ComputeShaderConstant constant = constants[name];
		location.offset = constant.offset;
		location.size = constant.size;
	}
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	ComputeTextureUnit unit;
	return unit;
}

void Compute::setBool(ComputeConstantLocation location, bool value) {}

void Compute::setInt(ComputeConstantLocation location, int value) {}

void Compute::setFloat(ComputeConstantLocation location, float value) {
	::setFloat(constantsMemory, location.offset, location.size, value);
}

void Compute::setFloat2(ComputeConstantLocation location, float value1, float value2) {
	::setFloat2(constantsMemory, location.offset, location.size, value1, value2);
}

void Compute::setFloat3(ComputeConstantLocation location, float value1, float value2, float value3) {
	::setFloat3(constantsMemory, location.offset, location.size, value1, value2, value3);
}

void Compute::setFloat4(ComputeConstantLocation location, float value1, float value2, float value3, float value4) {
	::setFloat4(constantsMemory, location.offset, location.size, value1, value2, value3, value4);
}

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {}

void Compute::setMatrix(ComputeConstantLocation location, const mat3& value) {}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {
	ID3D11ShaderResourceView* nullView = nullptr;
	context->PSSetShaderResources(0, 1, &nullView);

	context->CSSetUnorderedAccessViews(0, 1, &texture->computeView, nullptr);
}

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

void Compute::setShader(ComputeShader* shader) {
	context->CSSetShader((ID3D11ComputeShader*)shader->shader, nullptr, 0);

	context->UpdateSubresource(shader->constantBuffer, 0, nullptr, constantsMemory, 0, 0);
	context->CSSetConstantBuffers(0, 1, &shader->constantBuffer);
}

void Compute::compute(int x, int y, int z) {
	context->Dispatch(x, y, z);

	ID3D11UnorderedAccessView* nullView = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullView, nullptr);
}
