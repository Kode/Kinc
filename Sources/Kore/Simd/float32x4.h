#pragma once

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
