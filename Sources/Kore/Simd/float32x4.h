#pragma once

#include <kinc/simd/float32x4.h>

namespace Kore {
	typedef Kinc_float32x4 float32x4;

	inline float32x4 load(float a, float b, float c, float d) {
		return Kinc_load(a, b, c, d);
	}

	inline float32x4 loadAll(float t) {
		return Kinc_loadAll(t);
	}

	inline float get(float32x4 t, int index) {
		return Kinc_get(t, index);
	}

	inline float32x4 abs(float32x4 t) {
		return Kinc_abs(t);
	}

	inline float32x4 add(float32x4 a, float32x4 b) {
		return Kinc_add(a, b);
	}

	inline float32x4 div(float32x4 a, float32x4 b) {
		return Kinc_div(a, b);
	}

	inline float32x4 mul(float32x4 a, float32x4 b) {
		return Kinc_mul(a, b);
	}

	inline float32x4 neg(float32x4 t) {
		return Kinc_neg(t);
	}

	inline float32x4 reciprocalApproximation(float32x4 t) {
		return Kinc_reciprocalApproximation(t);
	}

	inline float32x4 reciprocalSqrtApproximation(float32x4 t) {
		return Kinc_reciprocalSqrtApproximation(t);
	}

	inline float32x4 sub(float32x4 a, float32x4 b) {
		return Kinc_sub(a, b);
	}

	inline float32x4 sqrt(float32x4 t) {
		return Kinc_sqrt(t);
	}
}
