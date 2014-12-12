#pragma once

#include "Matrix.h"

namespace Kore {
	class Quaternion {
	public:
		Quaternion();
		Quaternion(float x, float y, float z, float w);
		Quaternion(const vec3& axis, float radians);
		Quaternion slerp(float t, const Quaternion& q) const;
		Quaternion rotated(const Quaternion& b) const;
		Quaternion scaled(float scale) const;
		float dot(const Quaternion& q) const;
		mat4 matrix() const;
		Quaternion operator+(const Quaternion& q) const;
		Quaternion operator+(const vec3& v) const;
		void operator+=(const vec3& v);
		Quaternion operator*(const Quaternion& r) const;
		Quaternion operator-(const Quaternion& q) const;
		bool operator==(const Quaternion& q) const;
		bool operator!=(const Quaternion& q) const;
		void normalize();
		void rotate(const Quaternion& q2);

		

	private:
		float x, y, z, w;
	};
}
