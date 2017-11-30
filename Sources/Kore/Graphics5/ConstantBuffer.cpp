#include "pch.h"

#include "ConstantBuffer.h"

#include <Kore/Math/Matrix.h>

using namespace Kore;

namespace {
	void setInt(u8* constants, int offset, int value) {
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value;
	}

	void setFloat(u8* constants, int offset, float value) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value;
	}

	void setFloat2(u8* constants, int offset, float value1, float value2) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
	}

	void setFloat3(u8* constants, int offset, float value1, float value2, float value3) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
	}

	void setFloat4(u8* constants, int offset, float value1, float value2, float value3, float value4) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		floats[0] = value1;
		floats[1] = value2;
		floats[2] = value3;
		floats[3] = value4;
	}

	void setFloats(u8* constants, int offset, float* values, int count) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int i = 0; i < count; ++i) {
			floats[i] = values[i];
		}
	}

	void setBool(u8* constants, int offset, bool value) {
		int* ints = reinterpret_cast<int*>(&constants[offset]);
		ints[0] = value ? 1 : 0;
	}

	void setMatrix(u8* constants, int offset, const mat4& value) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 4; ++y) {
			for (int x = 0; x < 4; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}

	void setMatrix(u8* constants, int offset, const mat3& value) {
		float* floats = reinterpret_cast<float*>(&constants[offset]);
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				floats[x + y * 4] = value.get(y, x);
			}
		}
	}
}

void Graphics5::ConstantBuffer::setInt(int offset, int value) {
	::setInt(data, offset, value);
}

void Graphics5::ConstantBuffer::setFloat(int offset, float value) {
	::setFloat(data, offset, value);
}

void Graphics5::ConstantBuffer::setFloat2(int offset, float value1, float value2) {
	::setFloat2(data, offset, value1, value2);
}

void Graphics5::ConstantBuffer::setFloat2(int offset, vec2 value) {
	::setFloat2(data, offset, value.x(), value.y());
}

void Graphics5::ConstantBuffer::setFloat3(int offset, float value1, float value2, float value3) {
	::setFloat3(data, offset, value1, value2, value3);
}

void Graphics5::ConstantBuffer::setFloat3(int offset, vec3 value) {
	::setFloat3(data, offset, value.x(), value.y(), value.z());
}

void Graphics5::ConstantBuffer::setFloat4(int offset, float value1, float value2, float value3, float value4) {
	::setFloat4(data, offset, value1, value2, value3, value4);
}

void Graphics5::ConstantBuffer::setFloat4(int offset, vec4 value) {
	::setFloat4(data, offset, value.x(), value.y(), value.z(), value.w());
}

void Graphics5::ConstantBuffer::setFloats(int offset, float* values, int count) {
	::setFloats(data, offset, values, count);
}

void Graphics5::ConstantBuffer::setBool(int offset, bool value) {
	::setBool(data, offset, value);
}

void Graphics5::ConstantBuffer::setMatrix(int offset, const mat4& value) {
	::setMatrix(data, offset, value);
}

void Graphics5::ConstantBuffer::setMatrix(int offset, const mat3& value) {
	::setMatrix(data, offset, value);
}
