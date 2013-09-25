#pragma once

#include "Core.h"
#include "Vector.h"

namespace Kore {
	template<unsigned X, unsigned Y, class T> class Matrix {
		typedef Matrix<X, Y, T> myType;
	public:
		T matrix[X][Y];
	
		void Set(unsigned x, unsigned y, T t) { matrix[x][y] = t; }
	
		const static unsigned Width = X;
		const static unsigned Height = Y;

		class Setter {
			Matrix<X, Y, T>* m;
			unsigned x, y;
		public:
			Setter(Matrix<X, Y, T>* m, unsigned x, unsigned y) : m(m), x(x), y(y) {}
			T operator =(T t) { m->Set(x, y, t); return t; }
			operator T() { return m->matrix[x][y]; }
		};

		class RowSetter {
			Matrix<X, Y, T>* m;
			unsigned x;
		public:
			RowSetter(Matrix<X, Y, T>* m, unsigned x) : m(m), x(x) {}
			Setter operator [](unsigned i) { return Setter(m, x, i); }
		};
		friend class Matrix::Setter;
		friend class Matrix::RowSetter;

		float get(int x, int y) const {
			return matrix[x][y];
		}

		static myType Translation(float x, float y, float z) {
			//StaticAssert(X == 4 && Y == 4);
			myType m = Identity();
			m.Set(3, 0, x);
			m.Set(3, 1, y);
			m.Set(3, 2, z);
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
			m.Set(1, 1, cos(alpha));
			m.Set(2, 1, -sin(alpha));
			m.Set(1, 2, sin(alpha));
			m.Set(2, 2, cos(alpha));
			return m;
		}

		static myType RotationY(float alpha) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			m.Set(0, 0, cos(alpha));
			m.Set(2, 0, sin(alpha));
			m.Set(0, 2, -sin(alpha));
			m.Set(2, 2, cos(alpha));
			return m;
		}

		static myType RotationZ(float alpha) {
			//StaticAssert(X >= 3 && Y >= 3);
			myType m = Identity();
			m.Set(0, 0, cos(alpha));
			m.Set(1, 0, -sin(alpha));
			m.Set(0, 1, sin(alpha));
			m.Set(1, 1, cos(alpha));
			return m;
		}

		Matrix() {
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) matrix[x][y] = 0;
		}
		RowSetter operator [](unsigned i) { return RowSetter(this, i); }
		myType operator +(myType aMatrix) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] + aMatrix[x][y];
			return m;
		}
		myType operator -(myType aMatrix) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] - aMatrix[x][y];
			return m;
		}
		
		myType operator *(T t) {
			myType m;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) m[x][y] = matrix[x][y] * t;
			return m;
		}

		Matrix<Y, X, T> Transpose() const {
			Matrix<Y, X, T> transpose;
			for (unsigned x = 0; x < X; ++x) for (unsigned y = 0; y < Y; ++y) transpose[y][x] = matrix[x][y];
			return transpose;
		}
		
		Matrix<Y, X, T> Transpose3x3() const {
			//StaticAssert(X >= 3 && Y >= 3);
			Matrix<Y, X, T> transpose;
			for (unsigned x = 0; x < 3; ++x) for (unsigned y = 0; y < 3; ++y) transpose[y][x] = matrix[x][y];
			for (unsigned x = 3; x < X; ++x) for (unsigned y = 3; y < Y; ++y) transpose[x][y] = matrix[x][y];
			return transpose;
		}
		
		T Trace() const {
			//StaticAssert(X == Y);
			T t = 0;
			for (unsigned y = 0; y < Y; ++y) t += matrix[y][y];
			return t;
		}

		template<class M> Matrix<M::Width, Y, T> operator*(M m) const {
			//StaticAssert(X == M::Height);
			Matrix<M::Width, Y, T> product;
			for (unsigned x = 0; x < M::Width; ++x) for (unsigned y = 0; y < Y; ++y) {
				T t = 0;
				for (unsigned i = 0; i < X; ++i) t += matrix[i][y] * m[x][i];
				product[x][y] = t;
			}
			return product;
		}

		Vector<T, X> operator*(const Vector<T, X>& vec) const {
			Vector<T, X> product;
			for (unsigned y = 0; y < Y; ++y) {
				T t = 0;
				for (unsigned i = 0; i < X; ++i) t += matrix[i][y] * vec[i];
				product[y] = t;
			}
			return product;
		}

		void operator*=(Matrix<X, Y, T> m) {
			*this = *this * m;
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
					result.data[x][y] = a.data[x][y] * (1 - prop) + b.data[x][y] * prop;
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
			myType I;

			for (unsigned i = 0; i < X; ++i) I[i][i] = 1;
			for (unsigned j = 0; j < X; j++) {
				// Diagonalenfeld normalisieren
				q = matrix[j][j];
				if (q == 0) {
					//Gewährleisten, daß keine 0 in der Diagonale steht
					for (unsigned i = j + 1; i < X; ++i)
					{
						// Suche Reihe mit Feld <> 0 und addiere dazu
						if (matrix[j][i] != 0) {
							for (unsigned k = 0; k < X; ++k) {
								matrix[k][j] = matrix[k][j] + matrix[k][i];
								I[k][j] = I[k][j] + I[k][i];
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
						I[k][j] = I[k][j] / q;
					}
				}
				// Spalten außerhalb der Diagonalen auf 0 bringen
				for (unsigned i = 0; i < X; ++i) {
					if (i != j) {
						q = matrix[j][i];
						for (unsigned k = 0; k < X; ++k) {
							I[k][i] = I[k][i] - q * I[k][j];
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
}
