#include <kinc/backend/compute.h>

#include <kinc/compute/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include <kinc/backend/SystemMicrosoft.h>

static uint8_t constantsMemory[1024 * 4];

static int getMultipleOf16(int value) {
	int ret = 16;
	while (ret < value) ret += 16;
	return ret;
}

static void setInt(uint8_t *constants, uint32_t offset, uint32_t size, int value) {
	if (size == 0) return;
	int *ints = (int *)&constants[offset];
	ints[0] = value;
}

static void setFloat(uint8_t *constants, uint32_t offset, uint32_t size, float value) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	floats[0] = value;
}

static void setFloat2(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
}

static void setFloat3(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

static void setFloat4(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3, float value4) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

static void setFloats(uint8_t *constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, float *values, int count) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	if (columns == 4 && rows == 4) {
		for (int i = 0; i < count / 16 && i < (int)size / 4; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
				}
			}
		}
	}
	else if (columns == 3 && rows == 3) {
		for (int i = 0; i < count / 9 && i < (int)size / 3; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
				}
			}
		}
	}
	else if (columns == 2 && rows == 2) {
		for (int i = 0; i < count / 4 && i < (int)size / 2; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
				}
			}
		}
	}
	else {
		for (int i = 0; i < count && i * 4 < (int)size; ++i) {
			floats[i] = values[i];
		}
	}
}

static void setBool(uint8_t *constants, uint32_t offset, uint32_t size, bool value) {
	if (size == 0) return;
	int *ints = (int *)&constants[offset];
	ints[0] = value ? 1 : 0;
}

static void setMatrix4(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix4x4_t *value) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = kinc_matrix4x4_get(value, y, x);
		}
	}
}

static void setMatrix3(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix3x3_t *value) {
	if (size == 0) return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = kinc_matrix3x3_get(value, y, x);
		}
	}
}

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *_data, int length) {
	unsigned index = 0;
	uint8_t *data = (uint8_t *)_data;

	memset(&shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.attributes[i].hash = kinc_internal_hash_name(name);
		shader->impl.attributes[i].index = data[index++];
	}

	memset(&shader->impl.textures, 0, sizeof(shader->impl.textures));
	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		shader->impl.textures[i].hash = kinc_internal_hash_name(name);
		shader->impl.textures[i].index = data[index++];
	}

	memset(&shader->impl.constants, 0, sizeof(shader->impl.constants));
	uint8_t constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0) break;
		}
		kinc_compute_internal_shader_constant_t constant;
		constant.hash = kinc_internal_hash_name(name);
		constant.offset = *(uint32_t *)&data[index];
		index += 4;
		constant.size = *(uint32_t *)&data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;

		shader->impl.constants[i] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = (int)(length - index);
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	assert(shader->impl.data != NULL);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {0};
	desc.CS.BytecodeLength = shader->impl.length;
	desc.CS.pShaderBytecode = shader->impl.data;
	HRESULT hr = device->CreateComputePipelineState(&desc, IID_GRAPHICS_PPV_ARGS(&shader->impl.pso));

	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

	// kinc_microsoft_affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(shader->impl.constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
	// &shader->impl.constantBuffer));
}

void kinc_compute_shader_destroy(kinc_compute_shader_t *shader) {}

static kinc_compute_internal_shader_constant_t *findComputeConstant(kinc_compute_internal_shader_constant_t *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *findComputeTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name) {
	kinc_compute_constant_location_t location;

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_compute_internal_shader_constant_t *constant = findComputeConstant(shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.offset = 0;
		location.impl.size = 0;
		location.impl.columns = 0;
		location.impl.rows = 0;
	}
	else {
		location.impl.offset = constant->offset;
		location.impl.size = constant->size;
		location.impl.columns = constant->columns;
		location.impl.rows = constant->rows;
	}

	if (location.impl.size == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}

	return location;
}

kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63) len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_compute_texture_unit_t unit;
	kinc_internal_hash_index_t *vertexUnit = findComputeTextureUnit(shader->impl.textures, hash);
	if (vertexUnit == NULL) {
		unit.impl.unit = -1;
#ifndef NDEBUG
		static int notFoundCount = 0;
		if (notFoundCount < 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Sampler %s not found.", unitName);
			++notFoundCount;
		}
		else if (notFoundCount == 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Giving up on sampler not found messages.", unitName);
			++notFoundCount;
		}
#endif
	}
	else {
		unit.impl.unit = vertexUnit->index + unitOffset;
	}
	return unit;
}

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value) {
	setBool(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_int(kinc_compute_constant_location_t location, int value) {
	setInt(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_float(kinc_compute_constant_location_t location, float value) {
	setFloat(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2) {
	setFloat2(constantsMemory, location.impl.offset, location.impl.size, value1, value2);
}

void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3) {
	setFloat3(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3);
}

void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4) {
	setFloat4(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3, value4);
}

void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count) {
	setFloats(constantsMemory, location.impl.offset, location.impl.size, location.impl.columns, location.impl.rows, values, count);
}

void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, kinc_matrix4x4_t *value) {
	setMatrix4(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, kinc_matrix3x3_t *value) {
	setMatrix3(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, struct kinc_g4_texture *texture, kinc_compute_access_t access) {
	// ID3D12ShaderResourceView *nullView = nullptr;
	// context->PSSetShaderResources(0, 1, &nullView);

	// context->CSSetUnorderedAccessViews(unit.impl.unit, 1, &texture->impl.computeView, nullptr);
}

void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *texture, kinc_compute_access_t access) {}

void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, struct kinc_g4_texture *texture) {}

void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *target) {}

void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, struct kinc_g4_render_target *target) {}

void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, kinc_g4_texture_filter_t filter) {}

void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_compute_set_shader(kinc_compute_shader_t *shader) {
	ID3D12GraphicsCommandList *command_list = NULL;
	command_list->SetPipelineState(shader->impl.pso);

	// context->UpdateSubresource(shader->impl.constantBuffer, 0, nullptr, constantsMemory, 0, 0);
	// context->CSSetConstantBuffers(0, 1, &shader->impl.constantBuffer);
}

void kinc_compute(int x, int y, int z) {
	ID3D12GraphicsCommandList *command_list = NULL;
	// context->Dispatch(x, y, z);

	// ID3D11UnorderedAccessView *nullView = nullptr;
	// context->CSSetUnorderedAccessViews(0, 1, &nullView, nullptr);
}
