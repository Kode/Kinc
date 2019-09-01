#include "pch.h"

#include "ComputeImpl.h"
#include "Direct3D11.h"

#include <kinc/compute/compute.h>
#include <kinc/math/core.h>
#include <kinc/graphics4/texture.h>

#include <Kore/SystemMicrosoft.h>

using namespace Kore;

namespace {
	u8 constantsMemory[1024 * 4];

	int getMultipleOf16(int value) {
		int ret = 16;
		while (ret < value) ret += 16;
		return ret;
	}

	void setInt(u8 *constants, u32 offset, u32 size, int value) {
		if (size == 0) return;
		int *ints = reinterpret_cast<int *>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(u8 *constants, u32 offset, u32 size, float value) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8 *constants, u32 offset, u32 size, float value1, float value2) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8 *constants, u32 offset, u32 size, float value1, float value2, float value3) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8 *constants, u32 offset, u32 size, float value1, float value2, float value3, float value4) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(u8 *constants, u32 offset, u32 size, u8 columns, u8 rows, float *values, int count) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
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

	void setBool(u8 *constants, u32 offset, u32 size, bool value) {
		if (size == 0) return;
		int *ints = reinterpret_cast<int *>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8 *constants, u32 offset, u32 size, kinc_matrix4x4_t *value) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = kinc_matrix4x4_get(value, y, x);
			}
		}
	}

	void setMatrix(u8 *constants, u32 offset, u32 size, kinc_matrix3x3_t *value) {
		if (size == 0) return;
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = kinc_matrix3x3_get(value, y, x);
			}
		}
	}
}

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *_data, int length) {
	unsigned index = 0;
	u8 *data = (u8 *)_data;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.attributes[name] = data[index++];
	}

	u8 texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.textures[name] = data[index++];
	}

	u8 constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		kinc_compute_internal_shader_constant_t constant;
		constant.offset = data[index];
		index += 4;
		constant.size = data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;
		shader->impl.constants[name] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.data = &data[index];
	shader->impl.length = length - index;

	kinc_microsoft_affirm(device->CreateComputeShader(shader->impl.data, shader->impl.length, nullptr, (ID3D11ComputeShader **)&shader->impl.shader));

	kinc_microsoft_affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(shader->impl.constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
	                                           &shader->impl.constantBuffer));
}

void kinc_compute_shader_destroy(kinc_compute_shader_t *shader) {}

kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_constant_location_t location;
	if (shader->impl.constants.find(name) != shader->impl.constants.end()) {
		kinc_compute_internal_shader_constant_t constant = shader->impl.constants[name];
		location.impl.offset = constant.offset;
		location.impl.size = constant.size;
		location.impl.columns = constant.columns;
		location.impl.rows = constant.rows;
	}
	return location;
}

kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63) len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                // Check for array - mySampler[2]
		unitOffset = int(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                     // Strip array from name
	}

	kinc_compute_texture_unit_t unit;
	unit.impl.unit = shader->impl.textures[unitName] + unitOffset;
	return unit;
}

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value) {
	::setBool(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_int(kinc_compute_constant_location_t location, int value) {
	::setInt(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_float(kinc_compute_constant_location_t location, float value) {
	::setFloat(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2) {
	::setFloat2(constantsMemory, location.impl.offset, location.impl.size, value1, value2);
}

void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3) {
	::setFloat3(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3);
}

void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4) {
	::setFloat4(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3, value4);
}

void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count) {
	::setFloats(constantsMemory, location.impl.offset, location.impl.size, location.impl.columns, location.impl.rows, values, count);
}

void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, kinc_matrix4x4_t *value) {
	::setMatrix(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, kinc_matrix3x3_t *value) {
	::setMatrix(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture *texture, kinc_compute_access_t access) {
	ID3D11ShaderResourceView *nullView = nullptr;
	context->PSSetShaderResources(0, 1, &nullView);

	context->CSSetUnorderedAccessViews(unit.impl.unit, 1, &texture->impl.computeView, nullptr);
}

void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *texture, kinc_compute_access_t access) {}

void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture *texture) {}

void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *target) {}

void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *target) {}

void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_shader(kinc_compute_shader_t *shader) {
	context->CSSetShader((ID3D11ComputeShader *)shader->impl.shader, nullptr, 0);

	context->UpdateSubresource(shader->impl.constantBuffer, 0, nullptr, constantsMemory, 0, 0);
	context->CSSetConstantBuffers(0, 1, &shader->impl.constantBuffer);
}

void kinc_compute(int x, int y, int z) {
	context->Dispatch(x, y, z);

	ID3D11UnorderedAccessView *nullView = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nullView, nullptr);
}
