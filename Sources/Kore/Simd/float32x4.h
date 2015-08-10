#pragma once

#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1

#include <xmmintrin.h>

namespace Kore {
	typedef __m128 float32x4;

	inline float32x4 load(float a, float b, float c, float d) {
		return _mm_set_ps(d, c, b, a);
	}

	inline float32x4 loadAll(float t) {
		return _mm_set_ps1(t);
	}

	inline float get(float32x4 t, int index) {
		union {
			__m128 value;
			float elements[4];
		} converter;
		converter.value = t;
		return converter.elements[index];
	}

	inline float32x4 abs(float32x4 t) {
		__m128 mask = _mm_set_ps1(-0.f);
		return _mm_andnot_ps(mask, t);
	}

	inline float32x4 add(float32x4 a, float32x4 b) {
		return _mm_add_ps(a, b);
	}

	inline float32x4 div(float32x4 a, float32x4 b) {
		return _mm_div_ps(a, b);
	}
	
	inline float32x4 mul(float32x4 a, float32x4 b) {
		return _mm_mul_ps(a, b);
	}

	inline float32x4 neg(float32x4 t) {
		__m128 negative = _mm_set_ps1(-1.0f);
		return _mm_mul_ps(t, negative);
	}
	
	inline float32x4 reciprocalApproximation(float32x4 t) {
		return _mm_rcp_ps(t);
	}

	inline float32x4 reciprocalSqrtApproximation(float32x4 t) {
		return _mm_rsqrt_ps(t);
	}

	inline float32x4 sub(float32x4 a, float32x4 b) {
		return _mm_sub_ps(a, b);
	}

	inline float32x4 sqrt(float32x4 t) {
		return _mm_sqrt_ps(t);
	}
}

#else

#include <Kore/Math/Core.h>

namespace Kore {
	struct float32x4 {
		float values[4];
	};

	inline float32x4 load(float a, float b, float c, float d) {
		float32x4 value;
		value.values[0] = a;
		value.values[1] = b;
		value.values[2] = c;
		value.values[3] = d;
		return value;
	}

	inline float32x4 loadAll(float t) {
		float32x4 value;
		value.values[0] = t;
		value.values[1] = t;
		value.values[2] = t;
		value.values[3] = t;
		return value;
	}

	inline float get(float32x4 t, int index) {
		return t.values[index];
	}

	inline float32x4 abs(float32x4 t) {
		float32x4 value;
		value.values[0] = Kore::abs(t.values[0]);
		value.values[1] = Kore::abs(t.values[1]);
		value.values[2] = Kore::abs(t.values[2]);
		value.values[3] = Kore::abs(t.values[3]);
		return value;
	}

	inline float32x4 add(float32x4 a, float32x4 b) {
		float32x4 value;
		value.values[0] = a.values[0] + b.values[0];
		value.values[1] = a.values[1] + b.values[1];
		value.values[2] = a.values[2] + b.values[2];
		value.values[3] = a.values[3] + b.values[3];
		return value;
	}

	inline float32x4 div(float32x4 a, float32x4 b) {
		float32x4 value;
		value.values[0] = a.values[0] / b.values[0];
		value.values[1] = a.values[1] / b.values[1];
		value.values[2] = a.values[2] / b.values[2];
		value.values[3] = a.values[3] / b.values[3];
		return value;
	}

	inline float32x4 mul(float32x4 a, float32x4 b) {
		float32x4 value;
		value.values[0] = a.values[0] * b.values[0];
		value.values[1] = a.values[1] * b.values[1];
		value.values[2] = a.values[2] * b.values[2];
		value.values[3] = a.values[3] * b.values[3];
		return value;
	}

	inline float32x4 neg(float32x4 t) {
		float32x4 value;
		value.values[0] = -t.values[0];
		value.values[1] = -t.values[1];
		value.values[2] = -t.values[2];
		value.values[3] = -t.values[3];
		return value;
	}

	inline float32x4 reciprocalApproximation(float32x4 t) {
		float32x4 value;
		value.values[0] = 0;
		value.values[1] = 0;
		value.values[2] = 0;
		value.values[3] = 0;
		return value;
	}

	inline float32x4 reciprocalSqrtApproximation(float32x4 t) {
		float32x4 value;
		value.values[0] = 0;
		value.values[1] = 0;
		value.values[2] = 0;
		value.values[3] = 0;
		return value;
	}

	inline float32x4 sub(float32x4 a, float32x4 b) {
		float32x4 value;
		value.values[0] = a.values[0] - b.values[0];
		value.values[1] = a.values[1] - b.values[1];
		value.values[2] = a.values[2] - b.values[2];
		value.values[3] = a.values[3] - b.values[3];
		return value;
	}

	inline float32x4 sqrt(float32x4 t) {
		float32x4 value;
		value.values[0] = Kore::sqrt(t.values[0]);
		value.values[1] = Kore::sqrt(t.values[1]);
		value.values[2] = Kore::sqrt(t.values[2]);
		value.values[3] = Kore::sqrt(t.values[3]);
		return value;
	}
}

#endif
