#pragma once

#include "Core.h"
#include "Vector.h"

namespace Kore {
	template<unsigned X, unsigned Y, class T> class Matrix {
		typedef Matrix<X, Y, T> myType;
	public:
		const static unsigned Width = X;
		const static unsigned Height = Y;

		union {
			T matrix[X][Y];
			T data[X*Y];
		};

		inline float get(int row, int col) const {
			return matrix[col][row];
		}
		inline void Set(unsigned row, unsigned col, T t) { matrix[col][row] = t; }

		class RowGetter {
			const Matrix<X, Y, T> *m;
			unsigned row;
		public:
			RowGetter(Matrix<X, Y, T> const * m, unsigned row) : m(m), row(row) {}
			inline T operator[](unsigned col) const { return m->matrix[col][row]; }
		};
		class RowSetter {
			Matrix<X, Y, T>* m;
			unsigned row;
		public:
			RowSetter(Matrix<X, Y, T>* m, unsigned row) : m(m), row(row) {}
			void operator =(Vector<T, X> const & v) {
				for (unsigned x = 0; x < X; ++x) m->Set(row, x, v[x]);
			}
			inline T& operator[](unsigned col) { return m->matrix[col][row]; }
			inline T operator[](unsigned col) const { return m->matrix[col][row]; }
		};

		friend class Matrix::RowGetter;
		friend class Matrix::RowSetter;

		inline RowGetter operator[](unsigned row) const { return RowGetter(this, row); }
		inline RowSetter operator[](unsigned row) { return RowSetter(this, row); }


		static myType orthogonalProjection(float left, float right, float bottom, float top, float zn, float zf) {
			float tx = -(right + left) / (right - left);
			float ty = -(top + bottom) / (top - bottom);
			float tz = -(zf + zn) / (zf - zn);

			myType m = Identity();
			m.Set(0, 0, 2 / (right - left)); m.Set(1, 0, 0); m.Set(2, 0, 0); m.Set(3, 0, 0);
			m.Set(0, 1, 0); m.Set(1, 1, 2 / (top - bottom)); m.Set(2, 1, 0); m.Set(3, 1, 0);
			m.Set(0, 2, 0); m.Set(1, 2, 0); m.Set(2, 2, -2 / (zf - zn)); m.Set(3, 2, 0);
			m.Set(0, 3, tx); m.Set(1, 3, ty); m.Set(2, 3, tz); m.Set(3, 3, 1);
			return m;
		}

		static myType Perspective(float left, float right, float top, float bottom, float near, float far) {
			myType m;
			m.Set(0, 0, (2 * near) / (right - left));
			m.Set(0, 2, (right + left) / (right - left));
			m.Set(1, 1, (2 * near) / (top - bottom));
			m.Set(2, 1, (top + bottom) / (top - bottom));
			m.Set(2, 2, -((far + near) / (far - near)));
			m.Set(2, 3, -((2 * far*near) / (far - near)));
			m.Set(3, 2, -1);
			return m;
		}

		static myType Perspective(float fov, float aspect, float near, float far) {
			myType m;
			float uh = cot(fov / 2.0f);
			float uw = uh / aspect;
			m.Set(0, 0, uw);
			m.Set(1, 1, uh);
			m.Set(2, 2, ((far + near) / (far - near)));
			m.Set(2, 3, -((2 * far*near) / (far - near)));
			m.Set(3, 2, 1);
			return m;
		}

		static myType lookAt(vec3 eye, vec3 at, vec3 up) {
			vec3 zaxis = at - eye;
			zaxis.normalize();
			vec3 xaxis = up % zaxis;
			xaxis.normalize();
			vec3 yaxis = zaxis % xaxis;

			Matrix<4, 4, float> view;
			view.Set(0, 0, xaxis.x()); view.Set(0, 1, xaxis.y()); view.Set(0, 2, xaxis.z()); view.Set(0, 3, -xaxis.dot(eye));
			view.Set(1, 0, yaxis.x()); view.Set(1, 1, yaxis.y()); view.Set(1, 2, yaxis.z()); view.Set(1, 3, -yaxis.dot(eye));
			view.Set(2, 0, zaxis.x()); view.Set(2, 1, zaxis.y()); view.Set(2, 2, zaxis.z()); view.Set(2, 3, -zaxis.dot(eye));
			view.Set(3, 0, 0);         view.Set(3, 1, 0);         view.Set(3, 2, 0);         view.Set(3, 3, 1);
			return view;
		}

		static myType lookAlong(vec3 axis, vec3 eye, vec3 up) {
			vec3 zaxis = axis;
			zaxis.normalize();
			vec3 xaxis = up % zaxis;
			xaxis.normalize();
			vec3 yaxis = zaxis % xaxis;

			Matrix<4, 4, float> view;
			view.Set(0, 0, xaxis.x()); view.Set(0, 1, xaxis.y()); view.Set(0, 2, xaxis.z()); view.Set(0, 3, -xaxis.dot(eye));
			view.Set(1, 0, yaxis.x()); view.Set(1, 1, yaxis.y()); view.Set(1, 2, yaxis.z()); view.Set(1, 3, -yaxis.dot(eye));
			view.Set(2, 0, zaxis.x()); view.Set(2, 1, zaxis.y()); view.Set(2, 2, zaxis.z()); view.Set(2, 3, -zaxis.dot(eye));
			view.Set(3, 0, 0);         view.Set(3, 1, 0);         view.Set(3, 2, 0);         view.Set(3, 3, 1);
			return view;
		}

		static myType Translation(float x, float y, float z) {
			//StaticAssert(X == 4 && Y == 4);
			myType m = Identity();
			m.Set(0, X - 1, x);
			m.Set(1, X - 1, y);
			m.Set(2, X - 1, z);
			return m;
		}

		static myType Movement(float x, float y, float z) {
			return Translation(x, y, z);
		}

		static myType Identity() {
			//StaticAssert(X == Y);
			myType m;
			for (unsigned x = 0; x < X; ++x) m.Set(x, x, 1);
			return m;
		}

		static myType Scale(float x, float y, float z) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			m.Set(0, 0, x);
			m.Set(1, 1, y);
			m.Set(2, 2, z);
			return m;
		}

		static myType RotationX(float alpha) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			const float ca = cos(alpha);
			const float sa = sin(alpha);
			m.Set(1, 1,  ca);
			m.Set(1, 2, -sa);
			m.Set(2, 1,  sa);
			m.Set(2, 2,  ca);
			return m;
		}

		static myType RotationY(float alpha) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			const float ca = cos(alpha);
			const float sa = sin(alpha);
			m.Set(0, 0,  ca);
			m.Set(0, 2,  sa);
			m.Set(2, 0, -sa);
			m.Set(2, 2,  ca);
			return m;
		}

		static myType RotationZ(float alpha) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			const float ca = cos(alpha);
			const float sa = sin(alpha);
			m.Set(0, 0,  ca);
			m.Set(0, 1, -sa);
			m.Set(1, 0,  sa);
			m.Set(1, 1,  ca);
			return m;
		}

		static myType Rotation(float yaw, float pitch, float roll) {
			myType m = Identity();
			float sy = sin(yaw);
			float cy = cos(yaw);
			float sx = sin(pitch);
			float cx = cos(pitch);
			float sz = sin(roll);
			float cz = cos(roll);
			m.Set(0, 0, cx*cy);    m.Set(0, 1, cx*sy*sz - sx*cz);    m.Set(0, 2, cx*sy*cz + sx*sz);
			m.Set(1, 0, sx*cy);    m.Set(1, 1, sx*sy*sz + cx*cz);    m.Set(1, 2, sx*sy*cz - cx*sz);
			m.Set(2, 0, -sy);      m.Set(2, 1, cy*sz);               m.Set(2, 2, cy*cz);
			return m;
		}

		Matrix() {
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) matrix[x][y] = 0;
		}
		Matrix(myType const &other) {
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) matrix[x][y] = other.matrix[x][y];
		}
		explicit Matrix(Matrix<X + 1, Y + 1, T> const &other) {
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) matrix[x][y] = other.matrix[x][y];
		}
		explicit Matrix(Matrix<X - 1, Y - 1, T> const &other) {
			for (unsigned x = 0; x < X - 1; ++x) {
				for (unsigned y = 0; y < Y - 1; ++y) matrix[x][y] = other.matrix[x][y];
				matrix[x][Y - 1] = 0;
			}
			for (unsigned y = 0; y < Y - 1; ++y) matrix[X - 1][y] = 0;
			matrix[X - 1][Y - 1] = 1;
		}

		myType operator +(myType aMatrix) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] + aMatrix.matrix[x][y];
			return m;
		}
		myType operator -(myType aMatrix) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] - aMatrix.matrix[x][y];
			return m;
		}

		myType operator *(T t) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] * t;
			return m;
		}

		Matrix<Y, X, T> Transpose() const {
			Matrix<Y, X, T> transpose;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) transpose.matrix[y][x] = matrix[x][y];
			return transpose;
		}

		Matrix<Y, X, T> Transpose3x3() const {
			//StaticAssert(X >= 3 && Y >= 3);
			Matrix<Y, X, T> transpose;
			for (unsigned x = 0; x < 3; ++x) for (unsigned y = 0; y < 3; ++y) transpose.matrix[y][x] = matrix[x][y];
			for (unsigned x = 3; x < X; ++x) transpose.matrix[Y - 1][x] = matrix[x][Y - 1];
			for (unsigned y = 3; y < Y - 1; ++y) transpose.matrix[y][X - 1] = matrix[X - 1][y];
			return transpose;
		}

		T Trace() const {
			//StaticAssert(X == Y);
			T t = 0;
			for (unsigned y = 0; y < Y; ++y) t += matrix[y][y];
			return t;
		}

		template<unsigned X2> Matrix<X2, Y, T> operator*(const Matrix<X2, X, T>& m) const {
			Matrix<X2, Y, T> product;
			for (unsigned x = 0; x < X2; ++x) for (unsigned y = 0; y < Y; ++y) {
				T t = matrix[0][y] * m.matrix[x][0];
				for (unsigned i = 1; i < X; ++i) t += matrix[i][y] * m.matrix[x][i];
				product.matrix[x][y] = t;
			}
			return product;
		}

		template <unsigned S>
		Matrix<S, S, T>& operator*=(const Matrix<S, S, T>& m) {
			return *this = *this * m;
		}

		Matrix<3, 3, T>& operator*=(const Matrix<3, 3, T>& m) {
			for (unsigned y = 0; y < Y; ++y) {
				float a0 = matrix[0][y];
				float a1 = matrix[1][y];
				float a2 = matrix[2][y];
				matrix[0][y] = a0 * m.matrix[0][0] + a1 * m.matrix[0][1] + a2 * m.matrix[0][2];
				matrix[1][y] = a0 * m.matrix[1][0] + a1 * m.matrix[1][1] + a2 * m.matrix[1][2];
				matrix[2][y] = a0 * m.matrix[2][0] + a1 * m.matrix[2][1] + a2 * m.matrix[2][2];
			}
			return *this;
		}

		Matrix<4, 4, T>& operator*=(const Matrix<4, 4, T>& m) {
			for (unsigned y = 0; y < Y; ++y) {
				float a0 = matrix[0][y];
				float a1 = matrix[1][y];
				float a2 = matrix[2][y];
				float a3 = matrix[3][y];
				matrix[0][y] = a0 * m.matrix[0][0] + a1 * m.matrix[0][1] + a2 * m.matrix[0][2] + a3 * m.matrix[0][3];
				matrix[1][y] = a0 * m.matrix[1][0] + a1 * m.matrix[1][1] + a2 * m.matrix[1][2] + a3 * m.matrix[1][3];
				matrix[2][y] = a0 * m.matrix[2][0] + a1 * m.matrix[2][1] + a2 * m.matrix[2][2] + a3 * m.matrix[2][3];
				matrix[3][y] = a0 * m.matrix[3][0] + a1 * m.matrix[3][1] + a2 * m.matrix[3][2] + a3 * m.matrix[3][3];
			}
			return *this;
		}

		Vector<T, X> operator*(const Vector<T, X>& vec) const {
			Vector<T, X> product;
			for (unsigned y = 0; y < Y; ++y) {
				T t = 0;
				for (unsigned x = 0; x < X; ++x) t += matrix[x][y] * vec[x];
				product[y] = t;
			}
			return product;
		}

		T Determinant() {
			//StaticAssert(X == Y);
			if (X == 1) return matrix[0][0];
			if (X == 2) return matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1];
			if (X == 3) return
				  matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[2][1] * matrix[1][2])
				+ matrix[1][0] * (matrix[2][1] * matrix[0][2] - matrix[0][1] * matrix[2][2])
				+ matrix[2][0] * (matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]);
			if (X == 4) {
				return matrix[0][0] * (
					  matrix[1][1] * (matrix[2][2] * matrix[3][3] - matrix[3][2] * matrix[2][3])
					+ matrix[2][1] * (matrix[3][2] * matrix[1][3] - matrix[1][2] * matrix[3][3])
					+ matrix[3][1] * (matrix[1][2] * matrix[2][3] - matrix[2][2] * matrix[1][3])
				)
				- matrix[1][0] * (
					  matrix[0][1] * (matrix[2][2] * matrix[3][3] - matrix[3][2] * matrix[2][3])
					+ matrix[2][1] * (matrix[3][2] * matrix[0][3] - matrix[0][2] * matrix[3][3])
					+ matrix[3][1] * (matrix[0][2] * matrix[2][3] - matrix[2][2] * matrix[0][3])
				)
				+ matrix[2][0] * (
					  matrix[0][1] * (matrix[1][2] * matrix[3][3] - matrix[3][2] * matrix[1][3])
					+ matrix[1][1] * (matrix[3][2] * matrix[0][3] - matrix[0][2] * matrix[3][3])
					+ matrix[3][1] * (matrix[0][2] * matrix[1][3] - matrix[1][2] * matrix[0][3])
				)
				- matrix[3][0] * (
					  matrix[0][1] * (matrix[1][2] * matrix[2][3] - matrix[2][2] * matrix[1][3])
					+ matrix[1][1] * (matrix[2][2] * matrix[0][3] - matrix[0][2] * matrix[2][3])
					+ matrix[2][1] * (matrix[0][2] * matrix[1][3] - matrix[1][2] * matrix[0][3])
				);
			}
			myType a = detGLSL();
			T p = 1;
			for (unsigned i = 0; i < X; ++i) p *= a[i][i];
			return p;
		}

		static Matrix<Y, X, T> linearInterpolate(const Matrix<Y, X, T>& a, const Matrix<Y, X, T>& b, float prop) {
			Matrix<Y, X, T> result;
			for (unsigned x = 0; x < X; ++x)
				for (unsigned y = 0; y < Y; ++y)
					result.matrix[x][y] = a.matrix[x][y] * (1 - prop) + b.matrix[x][y] * prop;
			return result;
		}

	private:
		myType detGLSL() {
			T q;
			myType a = *this;
			for (unsigned j = 0; j < X; ++j) {
				q = a[j][j];
				if (q == 0) {
					for (unsigned i = j + 1; i < X; ++i) {
						if (a[j][i] != 0) {
							for (unsigned k = 0; k < X; ++k) a[k][j] = a[k][j] + a[k][i];
							q = a[j][j];
							break;
						}
					}
				}
				if (q != 0) {
					for (unsigned i = j + 1; i < X; ++i) {
						if(i != j) {
							q = a[j][i] / a[j][j];
							for (unsigned k = 0; k < X; ++k) a[k][i] = a[k][i] - q * a[k][j];
						}
					}
				}
				else return a;
			}
			return a;
		}
	public:
		void Invert() {
			//StaticAssert(X == Y);
			//if (Determinant() == 0) throw Exception(L"No Inverse");
			// m: Matrix, nz: Anzahl der Zeilen
			T q;
			myType I = myType::Identity();

			for (unsigned j = 0; j < X; ++j) {
				// Diagonalenfeld normalisieren
				q = matrix[j][j];
				if (q == 0) {
					//Gew�hrleisten, da� keine 0 in der Diagonale steht
					for (unsigned i = j + 1; i < X; ++i)
					{
						// Suche Reihe mit Feld <> 0 und addiere dazu
						if (matrix[j][i] != 0) {
							for (unsigned k = 0; k < X; ++k) {
								matrix[k][j] = matrix[k][j] + matrix[k][i];
								I.matrix[k][j] = I.matrix[k][j] + I.matrix[k][i];
							}
							q = matrix[j][j];
							break;
						}
					}
				}
				if (q != 0) {
					// Diagonalen auf 1 bringen
					for (unsigned k = 0; k < X; ++k) {
						matrix[k][j] = matrix[k][j] / q;
						I.matrix[k][j] = I.matrix[k][j] / q;
					}
				}
				// Spalten au�erhalb der Diagonalen auf 0 bringen
				for (unsigned i = 0; i < X; ++i) {
					if (i != j) {
						q = matrix[j][i];
						for (unsigned k = 0; k < X; ++k) {
							I.matrix[k][i] = I.matrix[k][i] - q * I.matrix[k][j];
							matrix[k][i] = matrix[k][i] - q * matrix[k][j];
						}
					}
				}
			}
			for (unsigned i = 0; i < X; ++i) for (unsigned j = 0; j < X; ++j) if (matrix[j][i] != ((i == j) ? 1 : 0));// throw Exception(L"Error");
			*this = I;
		}
	};

	typedef Matrix<2, 2, float> mat2;
	typedef Matrix<3, 3, float> mat3;
	typedef Matrix<4, 4, float> mat4;

	typedef Matrix<2, 2, double> dmat2;
	typedef Matrix<3, 3, double> dmat3;
	typedef Matrix<4, 4, double> dmat4;

	typedef Matrix<2, 2, float> mat2x2;
	typedef Matrix<2, 3, float> mat2x3;
	typedef Matrix<2, 4, float> mat2x4;
	typedef Matrix<3, 2, float> mat3x2;
	typedef Matrix<3, 3, float> mat3x3;
	typedef Matrix<3, 4, float> mat3x4;
	typedef Matrix<4, 2, float> mat4x2;
	typedef Matrix<4, 3, float> mat4x3;
	typedef Matrix<4, 4, float> mat4x4;

	typedef Matrix<2, 2, double> dmat2x2;
	typedef Matrix<2, 3, double> dmat2x3;
	typedef Matrix<2, 4, double> dmat2x4;
	typedef Matrix<3, 2, double> dmat3x2;
	typedef Matrix<3, 3, double> dmat3x3;
	typedef Matrix<3, 4, double> dmat3x4;
	typedef Matrix<4, 2, double> dmat4x2;
	typedef Matrix<4, 3, double> dmat4x3;
	typedef Matrix<4, 4, double> dmat4x4;

	/*
#ifndef _NO_CPP11_ // TODO: leave old c++ behind? In ten years maybe.
	static_assert(sizeof(mat3) == sizeof(float[3][3]), "Matrix4x4 does not match float[3][3] in size!");
	static_assert(sizeof(mat4) == sizeof(float[4][4]), "Matrix4x4 does not match float[4][4] in size!");
#else
// _STATIC_ASSERT from malloc.h
#ifndef _STATIC_ASSERT
#define _STATIC_ASSERT(expr) typedef char __static_assert_t[ (expr) ]
#define DO_STATIC_ASSERT_UNDEF_
#endif

	_STATIC_ASSERT(sizeof(mat4) == sizeof(float[4][4]));
	_STATIC_ASSERT(sizeof(int) == sizeof(Color));

#ifdef DO_STATIC_ASSERT_UNDEF_
#undef _STATIC_ASSERT
#endif
#endif
	 */
}
