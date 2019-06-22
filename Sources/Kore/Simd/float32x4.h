#pragma once

#include <kinc/simd/float32x4.h>

namespace Kore {
	typedef kinc_float32x4_t float32x4;

	inline float32x4 load(float a, float b, float c, float d) {
		return kinc_float32x4_load(a, b, c, d);
	}

	inline float32x4 loadAll(float t) {
		return kinc_float32x4_load_all(t);
	}

	inline float get(float32x4 t, int index) {
		return kinc_float32x4_get(t, index);
	}

	inline float32x4 abs(float32x4 t) {
		return kinc_float32x4_abs(t);
	}

	inline float32x4 add(float32x4 a, float32x4 b) {
		return kinc_float32x4_add(a, b);
	}

	inline float32x4 div(float32x4 a, float32x4 b) {
		return kinc_float32x4_div(a, b);
	}

	inline float32x4 mul(float32x4 a, float32x4 b) {
		return kinc_float32x4_mul(a, b);
	}

	inline float32x4 neg(float32x4 t) {
		return kinc_float32x4_neg(t);
	}

	inline float32x4 reciprocalApproximation(float32x4 t) {
		return kinc_float32x4_reciprocal_approximation(t);
	}

	inline float32x4 reciprocalSqrtApproximation(float32x4 t) {
		return kinc_float32x4_reciprocal_sqrt_approximation(t);
	}

	inline float32x4 sub(float32x4 a, float32x4 b) {
		return kinc_float32x4_sub(a, b);
	}

	inline float32x4 sqrt(float32x4 t) {
		return kinc_float32x4_sqrt(t);
	}
}
