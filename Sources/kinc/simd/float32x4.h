#pragma once

#include <kinc/global.h>

/*! \file float32x4.h
    \brief Provides 128bit four-element floating point SIMD operations which are mapped to equivalent SSE or Neon operations.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1 || defined(KORE_WINDOWS) || (defined(KORE_MACOS) && __x86_64)

#include <xmmintrin.h>

typedef __m128 kinc_float32x4_t;

inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	return _mm_set_ps(d, c, b, a);
}

inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	return _mm_set_ps1(t);
}

inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	union {
		__m128 value;
		float elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	__m128 mask = _mm_set_ps1(-0.f);
	return _mm_andnot_ps(mask, t);
}

inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_add_ps(a, b);
}

inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_div_ps(a, b);
}

inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_mul_ps(a, b);
}

inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	__m128 negative = _mm_set_ps1(-1.0f);
	return _mm_mul_ps(t, negative);
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	return _mm_rcp_ps(t);
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	return _mm_rsqrt_ps(t);
}

inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	return _mm_sub_ps(a, b);
}

inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
	return _mm_sqrt_ps(t);
}

#elif defined(KORE_IOS) || defined(KORE_SWITCH) || (defined(KORE_MACOS) && __arm64)

#include <arm_neon.h>

typedef float32x4_t kinc_float32x4_t;

inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	return {a, b, c, d};
}

inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	return {t, t, t, t};
}

inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	return t[index];
}

inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	return vabsq_f32(t);
}

inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vaddq_f32(a, b);
}

inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
#if defined(ARM64) || defined(KORE_SWITCH) || __arm64
	return vdivq_f32(a, b);
#else
	float32x4_t inv = vrecpeq_f32(b);
	float32x4_t restep = vrecpsq_f32(b, inv);
	inv = vmulq_f32(restep, inv);
	return vmulq_f32(a, inv);
#endif
}

inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vmulq_f32(a, b);
}

inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	return vnegq_f32(t);
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	return vrecpeq_f32(t);
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	return vrsqrteq_f32(t);
}

inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	return vsubq_f32(a, b);
}

inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
#if defined(ARM64) || defined(KORE_SWITCH) || __arm64
	return vsqrtq_f32(t);
#else
	return vmulq_f32(t, vrsqrteq_f32(t));
#endif
}

#else

#include <kinc/math/core.h>

typedef struct {
	float values[4];
} kinc_float32x4_t;

inline kinc_float32x4_t kinc_float32x4_load(float a, float b, float c, float d) {
	kinc_float32x4_t value;
	value.values[0] = a;
	value.values[1] = b;
	value.values[2] = c;
	value.values[3] = d;
	return value;
}

inline kinc_float32x4_t kinc_float32x4_load_all(float t) {
	kinc_float32x4_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

inline float kinc_float32x4_get(kinc_float32x4_t t, int index) {
	return t.values[index];
}

inline kinc_float32x4_t kinc_float32x4_abs(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = kinc_abs(t.values[0]);
	value.values[1] = kinc_abs(t.values[1]);
	value.values[2] = kinc_abs(t.values[2]);
	value.values[3] = kinc_abs(t.values[3]);
	return value;
}

inline kinc_float32x4_t kinc_float32x4_add(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

inline kinc_float32x4_t kinc_float32x4_div(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] / b.values[0];
	value.values[1] = a.values[1] / b.values[1];
	value.values[2] = a.values[2] / b.values[2];
	value.values[3] = a.values[3] / b.values[3];
	return value;
}

inline kinc_float32x4_t kinc_float32x4_mul(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] * b.values[0];
	value.values[1] = a.values[1] * b.values[1];
	value.values[2] = a.values[2] * b.values[2];
	value.values[3] = a.values[3] * b.values[3];
	return value;
}

inline kinc_float32x4_t kinc_float32x4_neg(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = -t.values[0];
	value.values[1] = -t.values[1];
	value.values[2] = -t.values[2];
	value.values[3] = -t.values[3];
	return value;
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_approximation(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = 0;
	value.values[1] = 0;
	value.values[2] = 0;
	value.values[3] = 0;
	return value;
}

inline kinc_float32x4_t kinc_float32x4_reciprocal_sqrt_approximation(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = 0;
	value.values[1] = 0;
	value.values[2] = 0;
	value.values[3] = 0;
	return value;
}

inline kinc_float32x4_t kinc_float32x4_sub(kinc_float32x4_t a, kinc_float32x4_t b) {
	kinc_float32x4_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

inline kinc_float32x4_t kinc_float32x4_sqrt(kinc_float32x4_t t) {
	kinc_float32x4_t value;
	value.values[0] = kinc_sqrt(t.values[0]);
	value.values[1] = kinc_sqrt(t.values[1]);
	value.values[2] = kinc_sqrt(t.values[2]);
	value.values[3] = kinc_sqrt(t.values[3]);
	return value;
}

#endif

#ifdef __cplusplus
}
#endif
