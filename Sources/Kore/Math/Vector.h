#pragma once

#include "Core.h"

namespace Kore {
	template<class Type, unsigned count> class Vector {
	public:
		Type values[count];
	
		Vector() {
			for (unsigned i = 0; i < count; ++i)
				values[i] = 0;
		}

		Vector(Type x, Type y) {
			set(x, y);
		}

		Vector(Type x, Type y, Type z) {
			set(x, y, z);
		}

		Vector(Type x, Type y, Type z, Type w) {
			set(x, y, z, w);
		}

		Vector(const Vector<Type, count - 1> other, Type w) {
			for (unsigned i = 0; i < count - 1; ++i) values[i] = other[i];
			values[count - 1] = w;
		}
		
		// construct new vector omitting last value
		Vector(const Vector<Type, count + 1>& other) {
			for (unsigned i = 0; i < count; ++i) values[i] = other[i];
		}
		
		/*explicit inline operator Vector<Type, count - 1>&() {
			return (Vector<Type, count - 1>&)*this;
		}
		
		explicit inline operator const Vector<Type, count - 1>&() const {
			return (const Vector<Type, count - 1>&)*this;
		}*/
		
		// Constructs cartesian vector from a homogeneous one
		Vector<Type, count - 1> toCartesian() {
			Vector<Type, count - 1> ret;
			float wInv = values[count - 1];
			if (wInv != 0.0f && wInv != 1.0f) { // TODO: eps
				wInv = 1 / wInv;
				for (unsigned i = 0; i < count - 1; ++i) ret[i] = values[i] * wInv;
			}
			else {
				for (unsigned i = 0; i < count - 1; ++i) ret[i] = values[i];
			}
			return ret;
		}

		void set(Type x, Type y) {
			//StaticAssert(count == 2);
			values[0] = x;
			values[1] = y;
		}

		void set(Type x, Type y, Type z) {
			//StaticAssert(count == 3);
			values[0] = x;
			values[1] = y;
			values[2] = z;
		}

		void set(Type x, Type y, Type z, Type w) {
			//StaticAssert(count == 4);
			values[0] = x;
			values[1] = y;
			values[2] = z;
			values[3] = w;
		}

		Type& x() {
			//StaticAssert(count > 0);
			return values[0];
		}

		Type& y() {
			//StaticAssert(count > 1);
			return values[1];
		}

		Type& z() {
			//StaticAssert(count > 2);
			return values[2];
		}

		Type& w() {
			//StaticAssert(count > 3);
			return values[3];
		}

		const Type& x() const {
			//StaticAssert(count > 0);
			return values[0];
		}

		const Type& y() const {
			//StaticAssert(count > 1);
			return values[1];
		}

		const Type& z() const {
			//StaticAssert(count > 2);
			return values[2];
		}

		const Type& w() const {
			//StaticAssert(count > 3);
			return values[3];
		}

		void add(Vector<Type, count> v) {
			for (unsigned i = 0; i < count; ++i)
				values[i] += v.values[i];
		}

		void addScaledVector(Vector<Type, count> v, float scale) {
			for (unsigned i = 0; i < count; ++i)
				values[i] += v.values[i] * scale;
		}

		void operator+=(Vector<Type, count> v) {
			add(v);
		}

		void sub(Vector<Type, count> v) {
			for (unsigned i = 0; i < count; ++i)
				values[i] -= v.values[i];
		}

		void operator-=(Vector<Type, count> v) {
			sub(v);
		}

		void multiply(Type value) {
			for (unsigned i = 0; i < count; ++i)
				values[i] *= value;
		}

		Vector<Type, count> componentProduct(const Vector<Type, count>& vector) const {
			Vector<Type, count> ret(*this);
			ret.componentProductUpdate(vector);
			return ret;
		}

		void componentProductUpdate(const Vector<Type, count>& vector) {
			for (unsigned i = 0; i < count; ++i) values[i] *= vector.values[i];
		}

		Vector<Type, count> operator-() {
			Vector<Type, count> ret;
			for (unsigned i = 0; i < count; ++i) ret.values[i] = -values[i];
			return ret;
		}

		void operator*=(Type value) {
			multiply(value);
		}

		void divide(Type value) {
			value = 1 / value;
			for (unsigned i = 0; i < count; ++i)
				values[i] *= value;
		}
		
		Vector<Type, count> operator/(Type value) {
			Vector<Type, count> ret(*this);
			ret.divide(value);
			return ret;
		}
		void operator/=(Type value) {
			divide(value);
		}

		Type squareLength() const {
			Type ret = 0;
			for (unsigned i = 0; i < count; ++i)
				ret += values[i] * values[i];
			return ret;
		}

		Type getLength() const {
			return sqrt(squareLength());
		}

		void setLength(Type length) {
			Type mul = length / getLength();
			for (unsigned i = 0; i < count; ++i)
				values[i] *= mul;
		}

		Vector<Type, count>& normalize() {
			setLength(1);
			return *this;
		}

		bool isZero() const {
			bool ret = true;
			for (unsigned i = 0; i < count; ++i)
				ret = ret && values[i] == 0;
			return ret;
		}

		Type& operator[](int index) {
			return values[index];
		}

		const Type& operator[](int index) const {
			return values[index];
		}

		void multiplyComponents(const Vector<Type, count>& v) {
			for (unsigned i = 0; i < count; ++i) values[i] = values[i] * v.values[i];
		}

		Type dot(Vector<Type, count> v) const {
			Type ret = 0;
			for (unsigned i = 0; i < count; ++i)
				ret += values[i] * v[i];
			return ret;
		}

		Vector<Type, count> cross(Vector<Type, count> v) const {
			//StaticAssert(count == 3);
			Type _x = y() * v.z() - z() * v.y();
			Type _y = z() * v.x() - x() * v.z();
			Type _z = x() * v.y() - y() * v.x();
			return Vector<Type, count>(_x, _y, _z);
		}

		Type distance(Vector<Type, count> v) {
			return (*this - v).getLength();
		}

		void invert() {
			for (unsigned i = 0; i < count; ++i) values[i] = -values[i];
		}
	};

	template<class Type, unsigned count> Vector<Type, count> operator+(Vector<Type, count> v1, Vector<Type, count> v2) {
		Vector<Type, count> ret(v1);
		ret.add(v2);
		return ret;
	}

	template<class Type, unsigned count> Vector<Type, count> operator-(Vector<Type, count> v1, Vector<Type, count> v2) {
		Vector<Type, count> ret(v1);
		ret.sub(v2);
		return ret;
	}

	template<class Type, unsigned count> Vector<Type, count> operator*(Vector<Type, count> v, Type f) {
		Vector<Type, count> ret(v);
		ret.multiply(f);
		return ret;
	}

	template<class Type, unsigned count> Vector<Type, count> operator*(Type f, Vector<Type, count> v) {
		return v * f;
	}

	template<class Type, unsigned count> Type operator*(Vector<Type, count> v1, Vector<Type, count> v2) {
		return v1.dot(v2);
	}

	template<class Type, unsigned count> Vector<Type, count> operator%(Vector<Type, count> v1, Vector<Type, count> v2) {
		return v1.cross(v2);
	}

	template<class Type, unsigned count> Vector<Type, count> operator/(Vector<Type, count> v, Type f) {
		Vector<Type, count> ret;
		ret.add(v);
		ret.divide(f);
		return ret;
	}

	template<class Type> bool operator==(Vector<Type, 1> v1, Vector<Type, 1> v2) {
		return v1.x() == v2.x();
	}

	template<class Type> bool operator==(Vector<Type, 2> v1, Vector<Type, 2> v2) {
		return v1.x() == v2.x() && v1.y() == v2.y();
	}

	template<class Type> bool operator==(Vector<Type, 3> v1, Vector<Type, 3> v2) {
		return v1.x() == v2.x() && v1.y() == v2.y() && v1.z() == v2.z();
	}

	template<class Type> bool operator==(Vector<Type, 4> v1, Vector<Type, 4> v2) {
		return v1.x() == v2.x() && v1.y() == v2.y() && v1.z() == v2.z() && v1.w() == v2.w();
	}

	template<class Type, unsigned count> bool operator!=(Vector<Type, count> v1, Vector<Type, count> v2) {
		return !(v1 == v2);
	}

	typedef Vector<float, 2> vec2;
	typedef Vector<float, 3> vec3;
	typedef Vector<float, 4> vec4;
	typedef Vector<double, 2> vec2d;
	typedef Vector<double, 3> vec3d;
	typedef Vector<double, 4> vec4d;
	typedef Vector<int, 2> vec2i;
	typedef Vector<int, 3> vec3i;
	typedef Vector<int, 4> vec4i;
}
