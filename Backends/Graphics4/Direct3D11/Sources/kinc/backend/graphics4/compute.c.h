#include "Direct3D11.h"

#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include <kinc/backend/SystemMicrosoft.h>

#define NOMINMAX

#ifdef KORE_WINDOWSAPP
#include <d3d11_1.h>
#else
#pragma warning(disable : 4005)
#include <d3d11.h>
#endif

#include <assert.h>

static uint8_t constantsMemory[1024 * 4];

static int getMultipleOf16(int value) {
	int ret = 16;
	while (ret < value)
		ret += 16;
	return ret;
}

static void compute_setInt(uint8_t *constants, uint32_t offset, uint32_t size, int value) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value;
}

static void compute_setFloat(uint8_t *constants, uint32_t offset, uint32_t size, float value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value;
}

static void compute_setFloat2(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
}

static void compute_setFloat3(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

static void compute_setFloat4(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3, float value4) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

static void compute_setFloats(uint8_t *constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, float *values, int count) {
	if (size == 0)
		return;
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

static void compute_setBool(uint8_t *constants, uint32_t offset, uint32_t size, bool value) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value ? 1 : 0;
}

static void compute_setMatrix4(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix4x4_t *value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = kinc_matrix4x4_get(value, y, x);
		}
	}
}

static void compute_setMatrix3(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix3x3_t *value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = kinc_matrix3x3_get(value, y, x);
		}
	}
}

void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *_data, int length) {
	unsigned index = 0;
	uint8_t *data = (uint8_t *)_data;

#ifndef KINC_KONG
	memset(&shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
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
			if (name[i2] == 0)
				break;
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
			if (name[i2] == 0)
				break;
		}
		kinc_g4_compute_internal_shader_constant constant;
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
#endif

	shader->impl.length = (int)(length - index);
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	assert(shader->impl.data != NULL);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	HRESULT hr =
	    dx_ctx.device->lpVtbl->CreateComputeShader(dx_ctx.device, shader->impl.data, shader->impl.length, NULL, (ID3D11ComputeShader **)&shader->impl.shader);

	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

#ifndef KINC_KONG
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = getMultipleOf16(shader->impl.constantsSize);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &shader->impl.constantBuffer));
#endif
}

void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader) {}

static kinc_g4_compute_internal_shader_constant *compute_findConstant(kinc_g4_compute_internal_shader_constant *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *compute_findTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

#ifndef KINC_KONG
kinc_g4_compute_constant_location kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_compute_constant_location location;

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_g4_compute_internal_shader_constant *constant = compute_findConstant(shader->impl.constants, hash);
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

kinc_g4_compute_texture_unit kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63)
		len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g4_compute_texture_unit unit;
	kinc_internal_hash_index_t *vertexUnit = compute_findTextureUnit(shader->impl.textures, hash);
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
#endif

void kinc_g4_compute_set_bool(kinc_g4_compute_constant_location location, bool value) {
	compute_setBool(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_g4_compute_set_int(kinc_g4_compute_constant_location location, int value) {
	compute_setInt(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_g4_compute_set_float(kinc_g4_compute_constant_location location, float value) {
	compute_setFloat(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_g4_compute_set_float2(kinc_g4_compute_constant_location location, float value1, float value2) {
	compute_setFloat2(constantsMemory, location.impl.offset, location.impl.size, value1, value2);
}

void kinc_g4_compute_set_float3(kinc_g4_compute_constant_location location, float value1, float value2, float value3) {
	compute_setFloat3(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3);
}

void kinc_g4_compute_set_float4(kinc_g4_compute_constant_location location, float value1, float value2, float value3, float value4) {
	compute_setFloat4(constantsMemory, location.impl.offset, location.impl.size, value1, value2, value3, value4);
}

void kinc_g4_compute_set_floats(kinc_g4_compute_constant_location location, float *values, int count) {
	compute_setFloats(constantsMemory, location.impl.offset, location.impl.size, location.impl.columns, location.impl.rows, values, count);
}

void kinc_g4_compute_set_matrix4(kinc_g4_compute_constant_location location, kinc_matrix4x4_t *value) {
	compute_setMatrix4(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_g4_compute_set_matrix3(kinc_g4_compute_constant_location location, kinc_matrix3x3_t *value) {
	compute_setMatrix3(constantsMemory, location.impl.offset, location.impl.size, value);
}

void kinc_g4_compute_set_texture(kinc_g4_compute_texture_unit unit, struct kinc_g4_texture *texture, kinc_g4_compute_access access) {
	ID3D11ShaderResourceView *nullView = NULL;
	dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, 0, 1, &nullView);

	dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, unit.impl.unit, 1, &texture->impl.computeView, NULL);
}

void kinc_g4_compute_set_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *texture, kinc_g4_compute_access access) {}

void kinc_g4_compute_set_sampled_texture(kinc_g4_compute_texture_unit unit, struct kinc_g4_texture *texture) {}

void kinc_g4_compute_set_sampled_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *target) {}

void kinc_g4_compute_set_sampled_depth_from_render_target(kinc_g4_compute_texture_unit unit, struct kinc_g4_render_target *target) {}

void kinc_g4_compute_set_texture_addressing(kinc_g4_compute_texture_unit unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_g4_compute_set_texture_magnification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter) {}

void kinc_g4_compute_set_texture_minification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter) {}

void kinc_g4_compute_set_texture_mipmap_filter(kinc_g4_compute_texture_unit unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_g4_compute_set_texture3d_addressing(kinc_g4_compute_texture_unit unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {}

void kinc_g4_compute_set_texture3d_magnification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter) {}

void kinc_g4_compute_set_texture3d_minification_filter(kinc_g4_compute_texture_unit unit, kinc_g4_texture_filter_t filter) {}

void kinc_g4_compute_set_texture3d_mipmap_filter(kinc_g4_compute_texture_unit unit, kinc_g4_mipmap_filter_t filter) {}

void kinc_g4_compute_set_shader(kinc_g4_compute_shader *shader) {
	dx_ctx.context->lpVtbl->CSSetShader(dx_ctx.context, (ID3D11ComputeShader *)shader->impl.shader, NULL, 0);
#ifndef KINC_KONG
	dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)shader->impl.constantBuffer, 0, NULL, constantsMemory, 0, 0);
	dx_ctx.context->lpVtbl->CSSetConstantBuffers(dx_ctx.context, 0, 1, &shader->impl.constantBuffer);
#endif
}

void kinc_g4_compute(int x, int y, int z) {
	dx_ctx.context->lpVtbl->Dispatch(dx_ctx.context, x, y, z);

	ID3D11UnorderedAccessView *nullView = NULL;
	dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, 0, 1, &nullView, NULL);
}
