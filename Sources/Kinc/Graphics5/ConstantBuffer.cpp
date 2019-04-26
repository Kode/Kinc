#include "pch.h"

#include "ConstantBuffer.h"

namespace {
	void setInt(uint8_t* constants, int offset, int value) {
		int *ints = reinterpret_cast<int *>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(uint8_t *constants, int offset, float value) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(uint8_t *constants, int offset, float value1, float value2) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(uint8_t *constants, int offset, float value1, float value2, float value3) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(uint8_t *constants, int offset, float value1, float value2, float value3, float value4) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(uint8_t *constants, int offset, float *values, int count) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}

	void setBool(uint8_t *constants, int offset, bool value) {
		int *ints = reinterpret_cast<int *>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(uint8_t *constants, int offset, Kinc_Matrix4x4 *value) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = kinc_matrix4x4_get(value, y, x);
			}
		}
	}

	void setMatrix(uint8_t *constants, int offset, Kinc_Matrix3x3 *value) {
		float *floats = reinterpret_cast<float *>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = kinc_matrix3x3_get(value, y, x);
			}
		}
	}
}

void kinc_g5_constant_buffer_set_int(kinc_g5_constant_buffer *buffer, int offset, int value) {
	setInt(buffer->data, offset, value);
}

void kinc_g5_constant_buffer_setFloat(kinc_g5_constant_buffer *buffer, int offset, float value) {
	setFloat(buffer->data, offset, value);
}

void kinc_g5_constant_buffer_setFloat2(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2) {
	setFloat2(buffer->data, offset, value1, value2);
}

void kinc_g5_constant_buffer_setFloat3(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2, float value3) {
	setFloat3(buffer->data, offset, value1, value2, value3);
}

void kinc_g5_constant_buffer_setFloat4(kinc_g5_constant_buffer *buffer, int offset, float value1, float value2, float value3, float value4) {
	setFloat4(buffer->data, offset, value1, value2, value3, value4);
}

void kinc_g5_constant_buffer_setFloats(kinc_g5_constant_buffer *buffer, int offset, float *values, int count) {
	setFloats(buffer->data, offset, values, count);
}

void kinc_g5_constant_buffer_set_bool(kinc_g5_constant_buffer *buffer, int offset, bool value) {
	setBool(buffer->data, offset, value);
}

void kinc_g5_constant_buffer_set_matrix4(kinc_g5_constant_buffer *buffer, int offset, Kinc_Matrix4x4 *value) {
	if (transposeMat4) {
		Kinc_Matrix4x4 m = *value;
		kinc_matrix4x4_transpose(&m);
		setMatrix(buffer->data, offset, &m);
	}
	else {
		setMatrix(buffer->data, offset, value);
	}
}

void kinc_g5_constant_buffer_set_matrix3(kinc_g5_constant_buffer *buffer, int offset, Kinc_Matrix3x3 *value) {
	if (transposeMat3) {
		Kinc_Matrix3x3 m = *value;
		kinc_matrix3x3_transpose(&m);
		setMatrix(buffer->data, offset, &m);
	}
	else {
		setMatrix(buffer->data, offset, value);
	}
}
