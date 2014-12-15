#include "pch.h"
#include "Quaternion.h"
#include "Core.h"

using namespace Kore;

Quaternion::Quaternion() : x(0), y(0), z(0), w(0) { }

Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }

Quaternion::Quaternion(const vec3& axis, float radians) {
	w = cos(radians / 2);
	x = y = z = sin(radians / 2);
	x *= axis.x();
	y *= axis.y();
	z *= axis.z();
}

Quaternion Quaternion::slerp(float t, const Quaternion& v1) const {
	const float epsilon = 0.0005f;
	float dot = Quaternion::dot(v1);

	if (dot > 1 - epsilon) {
		Quaternion result = v1 + (*this - v1).scaled(t);
		result.normalize();
		return result;
	}
	if (dot < 0) dot = 0;
	if (dot > 1) dot = 1;

	float theta0 = acos(dot);
	float theta = theta0 * t;

	Quaternion v2 = (v1 - scaled(dot));
	v2.normalize();

	Quaternion q = scaled(cos(theta)) + v2.scaled(sin(theta));
	q.normalize();

	return q;
}

Quaternion Quaternion::rotated(const Quaternion& b) const {
	Quaternion q;
	q.w = w * b.w - x * b.x - y * b.y - z * b.z;
	q.x = w * b.x + x * b.w + y * b.z - z * b.y;
	q.y = w * b.y + y * b.w + z * b.x - x * b.z;
	q.z = w * b.z + z * b.w + x * b.y - y * b.x;
	q.normalize();
	return q;
}

Quaternion Quaternion::scaled(float scale) const {
	return Quaternion(x * scale, y * scale, z * scale, w * scale);
}

float Quaternion::dot(const Quaternion& q) const {
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

mat4 Quaternion::matrix() const {
	const float s = 2;
	float xs = x * s; float ys = y * s; float zs = z * s;
	float wx = w * xs; float wy = w * ys; float wz = w * zs;
	float xx = x * xs; float xy = x * ys; float xz = x * zs;
	float yy = y * ys; float yz = y * zs; float zz = z * zs;
	mat4 m = mat4::Identity();
	m.Set(0, 0, 1 - (yy + zz)); m.Set(1, 0, xy - wz); m.Set(2, 0, xz + wy);
	m.Set(0, 1, xy + wz); m.Set(1, 1, 1 - (xx + zz)); m.Set(2, 1, yz - wx);
	m.Set(0, 2, xz - wy); m.Set(1, 2, yz + wx); m.Set(2, 2, 1 - (xx + yy));
	return m;
}

Quaternion Quaternion::operator-(const Quaternion& q) const {
	return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
}

Quaternion Quaternion::operator+(const Quaternion& q) const {
	return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
}

Quaternion Quaternion::operator+(const vec3& v) const {
	Quaternion result(x, y, z, w); 
	Quaternion q1(0,
                v.x(),
                v.y(), 
                v.z());
            q1 = q1 * result;
			result.x += q1.x * 0.5f;
			result.y += q1.y * 0.5f;
			result.z += q1.z * 0.5f;
			result.w += q1.w * 0.5f;
			return result;
}

void Quaternion::operator+=(const vec3& v) {
	Quaternion q(0, v.x(), v.y(), v.z());
	rotate(q);
	x += q.x * 0.5f;
	y += q.y * 0.5f;
	z += q.z * 0.5f;
	w += q.w * 0.5f;
}

Quaternion Quaternion::operator*(const Quaternion& r) const {
	Quaternion q;
	q.x = x * r.x - y * r.y - z * r.z - w * r.w;
	q.y = x * r.y + y * r.x - z * r.w + w * r.z;
	q.z = x * r.z - y * r.w + z * r.x - w * r.y;
	q.w = x * r.w + y * r.z - z * r.y + w * r.x;
	return q;
}



bool Quaternion::operator==(const Quaternion& q) const {
	return x == q.x && y == q.y && z == q.z && w == q.w;
}

bool Quaternion::operator!=(const Quaternion& q) const {
	return !(*this == q);
}

void Quaternion::normalize() {
	*this = scaled(1 / sqrt(dot(*this)));
}

void Quaternion::rotate(const Quaternion& q2) {
	Quaternion q;
	Quaternion& q1 = *this;

	q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
	q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	q.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
	q.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;

	// q.normalize();
	*this = q;
}
