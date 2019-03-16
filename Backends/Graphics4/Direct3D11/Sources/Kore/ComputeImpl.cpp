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

	void setInt(u8* constants, u32 offset, u32 size, int value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
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

	void setFloats(u8* constants, u32 offset, u32 size, u8 columns, u8 rows, float* values, int count) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		if (columns == 4 && rows == 4) {
			for (int i = 0; i < count / 16 && i < static_cast<int>(size) / 4; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
					}
				}
			}
		}
		else if (columns == 3 && rows == 3) {
			for (int i = 0; i < count / 9 && i < static_cast<int>(size) / 3; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
					}
				}
			}
		}
		else if (columns == 2 && rows == 2) {
			for (int i = 0; i < count / 4 && i < static_cast<int>(size) / 2; ++i) {
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						floats[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
					}
				}
			}
		}
		else {
			for (int i = 0; i < count && i * 4 < static_cast<int>(size); ++i) {
				floats[i] = values[i];
			}
		}
	}

	void setBool(u8* constants, u32 offset, u32 size, bool value) {
		if (size == 0) return;
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat4& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}

	void setMatrix(u8* constants, u32 offset, u32 size, const mat3& value) {
		if (size == 0) return;
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
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

	Kore_Microsoft_Affirm(device->CreateComputeShader(this->data, this->length, nullptr, (ID3D11ComputeShader**)&shader));

	Kore_Microsoft_Affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr, &constantBuffer));
}

ComputeConstantLocation ComputeShader::getConstantLocation(const char* name) {
	ComputeConstantLocation location;
	if (constants.find(name) != constants.end()) {
		ComputeShaderConstant constant = constants[name];
		location.offset = constant.offset;
		location.size = constant.size;
		location.columns = constant.columns;
		location.rows = constant.rows;
	}
	return location;
}

ComputeTextureUnit ComputeShader::getTextureUnit(const char* name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63) len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') { // Check for array - mySampler[2]
		unitOffset = int(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0; // Strip array from name
	}

	ComputeTextureUnit unit;
	unit.unit = textures[unitName] + unitOffset;
	return unit;
}

void Compute::setBool(ComputeConstantLocation location, bool value) {
	::setBool(constantsMemory, location.offset, location.size, value);
}

void Compute::setInt(ComputeConstantLocation location, int value) {
	::setInt(constantsMemory, location.offset, location.size, value);
}

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

void Compute::setFloats(ComputeConstantLocation location, float* values, int count) {
	::setFloats(constantsMemory, location.offset, location.size, location.columns, location.rows, values, count);
}

void Compute::setMatrix(ComputeConstantLocation location, const mat4& value) {
	::setMatrix(constantsMemory, location.offset, location.size, value);
}

void Compute::setMatrix(ComputeConstantLocation location, const mat3& value) {
	::setMatrix(constantsMemory, location.offset, location.size, value);
}

void Compute::setTexture(ComputeTextureUnit unit, Graphics4::Texture* texture, Access access) {
	ID3D11ShaderResourceView* nullView = nullptr;
	context->PSSetShaderResources(0, 1, &nullView);

	context->CSSetUnorderedAccessViews(unit.unit, 1, &texture->computeView, nullptr);
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
