#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1

#include <xmmintrin.h>

typedef __m128 Kinc_float32x4;

inline Kinc_float32x4 Kinc_load(float a, float b, float c, float d) {
	return _mm_set_ps(d, c, b, a);
}

inline Kinc_float32x4 Kinc_loadAll(float t) {
	return _mm_set_ps1(t);
}

inline float Kinc_get(Kinc_float32x4 t, int index) {
	union {
		__m128 value;
		float elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

inline Kinc_float32x4 Kinc_abs(Kinc_float32x4 t) {
	__m128 mask = _mm_set_ps1(-0.f);
	return _mm_andnot_ps(mask, t);
}

inline Kinc_float32x4 Kinc_add(Kinc_float32x4 a, Kinc_float32x4 b) {
	return _mm_add_ps(a, b);
}

inline Kinc_float32x4 Kinc_div(Kinc_float32x4 a, Kinc_float32x4 b) {
	return _mm_div_ps(a, b);
}

inline Kinc_float32x4 Kinc_mul(Kinc_float32x4 a, Kinc_float32x4 b) {
	return _mm_mul_ps(a, b);
}

inline Kinc_float32x4 Kinc_neg(Kinc_float32x4 t) {
	__m128 negative = _mm_set_ps1(-1.0f);
	return _mm_mul_ps(t, negative);
}

inline Kinc_float32x4 Kinc_reciprocalApproximation(Kinc_float32x4 t) {
	return _mm_rcp_ps(t);
}

inline Kinc_float32x4 Kinc_reciprocalSqrtApproximation(Kinc_float32x4 t) {
	return _mm_rsqrt_ps(t);
}

inline Kinc_float32x4 Kinc_sub(Kinc_float32x4 a, Kinc_float32x4 b) {
	return _mm_sub_ps(a, b);
}

inline Kinc_float32x4 Kinc_sqrt(Kinc_float32x4 t) {
	return _mm_sqrt_ps(t);
}

#elif defined(KORE_IOS) || defined(KORE_SWITCH)

#include <arm_neon.h>

typedef float32x4_t Kinc_float32x4;

inline Kinc_float32x4 Kinc_load(float a, float b, float c, float d) {
	return {a, b, c, d};
}

inline Kinc_float32x4 Kinc_loadAll(float t) {
	return {t, t, t, t};
}

inline float Kinc_get(Kinc_float32x4 t, int index) {
	return t[index];
}

inline Kinc_float32x4 Kinc_abs(Kinc_float32x4 t) {
	return vabsq_f32(t);
}

inline Kinc_float32x4 Kinc_add(Kinc_float32x4 a, Kinc_float32x4 b) {
	return vaddq_f32(a, b);
}

inline Kinc_float32x4 Kinc_div(Kinc_float32x4 a, Kinc_float32x4 b) {
#if defined(ARM64) || defined(KORE_SWITCH)
	return vdivq_f32(a, b);
#else
	float32x4 inv = vrecpeq_f32(b);
	float32x4 restep = vrecpsq_f32(b, inv);
	inv = vmulq_f32(restep, inv);
	return vmulq_f32(a, inv);
#endif
}

inline Kinc_float32x4 Kinc_mul(Kinc_float32x4 a, Kinc_float32x4 b) {
	return vmulq_f32(a, b);
}

inline Kinc_float32x4 Kinc_neg(Kinc_float32x4 t) {
	return vnegq_f32(t);
}

inline Kinc_float32x4 Kinc_reciprocalApproximation(Kinc_float32x4 t) {
	return vrecpeq_f32(t);
}

inline Kinc_float32x4 Kinc_reciprocalSqrtApproximation(Kinc_float32x4 t) {
	return vrsqrteq_f32(t);
}

inline Kinc_float32x4 Kinc_sub(Kinc_float32x4 a, Kinc_float32x4 b) {
	return vsubq_f32(a, b);
}

inline Kinc_float32x4 Kinc_sqrt(Kinc_float32x4 t) {
#if defined(ARM64) || defined(KORE_SWITCH)
	return vsqrtq_f32(t);
#else
	return vmulq_f32(t, vrsqrteq_f32(t));
#endif
}

#else

#include <Kore/Math/Core.h>

struct Kinc_float32x4 {
	float values[4];
};

inline Kinc_float32x4 Kinc_load(float a, float b, float c, float d) {
	Kinc_float32x4 value;
	value.values[0] = a;
	value.values[1] = b;
	value.values[2] = c;
	value.values[3] = d;
	return value;
}

inline Kinc_float32x4 Kinc_loadAll(float t) {
	Kinc_float32x4 value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

inline float Kinc_get(Kinc_float32x4 t, int index) {
	return t.values[index];
}

inline Kinc_float32x4 Kinc_abs(Kinc_float32x4 t) {
	Kinc_float32x4 value;
	value.values[0] = Kore::abs(t.values[0]);
	value.values[1] = Kore::abs(t.values[1]);
	value.values[2] = Kore::abs(t.values[2]);
	value.values[3] = Kore::abs(t.values[3]);
	return value;
}

inline Kinc_float32x4 Kinc_add(Kinc_float32x4 a, Kinc_float32x4 b) {
	Kinc_float32x4 value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

inline Kinc_float32x4 Kinc_div(Kinc_float32x4 a, Kinc_float32x4 b) {
	Kinc_float32x4 value;
	value.values[0] = a.values[0] / b.values[0];
	value.values[1] = a.values[1] / b.values[1];
	value.values[2] = a.values[2] / b.values[2];
	value.values[3] = a.values[3] / b.values[3];
	return value;
}

inline Kinc_float32x4 Kinc_mul(Kinc_float32x4 a, Kinc_float32x4 b) {
	Kinc_float32x4 value;
	value.values[0] = a.values[0] * b.values[0];
	value.values[1] = a.values[1] * b.values[1];
	value.values[2] = a.values[2] * b.values[2];
	value.values[3] = a.values[3] * b.values[3];
	return value;
}

inline Kinc_float32x4 Kinc_neg(Kinc_float32x4 t) {
	Kinc_float32x4 value;
	value.values[0] = -t.values[0];
	value.values[1] = -t.values[1];
	value.values[2] = -t.values[2];
	value.values[3] = -t.values[3];
	return value;
}

inline Kinc_float32x4 Kinc_reciprocalApproximation(Kinc_float32x4 t) {
	Kinc_float32x4 value;
	value.values[0] = 0;
	value.values[1] = 0;
	value.values[2] = 0;
	value.values[3] = 0;
	return value;
}

inline Kinc_float32x4 Kinc_reciprocalSqrtApproximation(Kinc_float32x4 t) {
	Kinc_float32x4 value;
	value.values[0] = 0;
	value.values[1] = 0;
	value.values[2] = 0;
	value.values[3] = 0;
	return value;
}

inline Kinc_float32x4 Kinc_sub(Kinc_float32x4 a, Kinc_float32x4 b) {
	Kinc_float32x4 value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

inline Kinc_float32x4 Kinc_sqrt(Kinc_float32x4 t) {
	Kinc_float32x4 value;
	value.values[0] = Kore::sqrt(t.values[0]);
	value.values[1] = Kore::sqrt(t.values[1]);
	value.values[2] = Kore::sqrt(t.values[2]);
	value.values[3] = Kore::sqrt(t.values[3]);
	return value;
}

#endif

#ifdef __cplusplus
}
#endif
