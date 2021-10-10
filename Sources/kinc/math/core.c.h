#include "core.h"

#include <float.h>

#ifdef KINC_NO_CLIB
#include <intrin.h>
#define USE_SSE2
#include <kinc/libs/sse_mathfun.h>
#else
#include <math.h>
#include <stdlib.h>
#endif

float kinc_tan(float x) {
	return tanf(x);
}

float kinc_cot(float x) {
#ifdef KINC_NO_CLIB
	return kinc_cos(x) / kinc_sin(x);
#else
	return cosf(x) / sinf(x);
#endif
}

float kinc_round(float value) {
#ifdef KINC_NO_CLIB
	return (float)(int)(value + 0.5f);
#else
	return floorf(value + 0.5f);
#endif
}

float kinc_ceil(float value) {
#ifdef KINC_NO_CLIB
	return (float)(int)(value + 1.0f);
#else
	return ceilf(value);
#endif
}

float kinc_floor(float value) {
#ifdef KINC_NO_CLIB
	return (float)(int)value;
#else
	return floorf(value);
#endif
}

float kinc_mod(float numer, float denom) {
#ifdef KINC_NO_CLIB
	__m128 ssenumer;
	ssenumer.m128_f32[0] = numer;
	__m128 ssedenom;
	ssedenom.m128_f32[0] = denom;
	return _mm_fmod_ps(ssenumer, ssedenom).m128_f32[0];
#else
	return fmodf(numer, denom);
#endif
}

float kinc_exp(float exponent) {
#ifdef KINC_NO_CLIB
	__m128 input;
	input.m128_f32[0] = exponent;
	__m128 output = exp_ps(input);
	return output.m128_f32[0];
#else
	return expf(exponent);
#endif
}

float kinc_pow(float value, float exponent) {
#ifdef KINC_NO_CLIB
	__m128 ssevalue;
	ssevalue.m128_f32[0] = value;
	__m128 sseexponent;
	sseexponent.m128_f32[0] = exponent;
	return _mm_pow_ps(ssevalue, sseexponent).m128_f32[0];
#else
	return powf(value, exponent);
#endif
}

float kinc_max_float() {
	return FLT_MAX;
}

float kinc_sqrt(float value) {
#ifdef KINC_NO_CLIB
	__m128 input;
	input.m128_f32[0] = value;
	return _mm_sqrt_ss(input).m128_f32[0];
#else
	return sqrtf(value);
#endif
}

float kinc_abs(float value) {
	return value < 0 ? -value : value;
}

float kinc_sin(float value) {
#ifdef KINC_NO_CLIB
	__m128 input;
	input.m128_f32[0] = value;
	__m128 output = sin_ps(input);
	return output.m128_f32[0];
#else
	return sinf(value);
#endif
}

float kinc_cos(float value) {
#ifdef KINC_NO_CLIB
	__m128 input;
	input.m128_f32[0] = value;
	__m128 output = cos_ps(input);
	return output.m128_f32[0];
#else
	return cosf(value);
#endif
}

float kinc_asin(float value) {
	return asinf(value);
}

float kinc_acos(float value) {
	return acosf(value);
}

float kinc_atan(float value) {
	return atanf(value);
}

float kinc_atan2(float y, float x) {
	return atan2f(y, x);
}

float kinc_min(float a, float b) {
	return a > b ? b : a;
}

float kinc_max(float a, float b) {
	return a > b ? a : b;
}

int kinc_mini(int a, int b) {
	return a > b ? b : a;
}

int kinc_maxi(int a, int b) {
	return a > b ? a : b;
}

float kinc_clamp(float value, float minValue, float maxValue) {
	return kinc_max(minValue, kinc_min(maxValue, value));
}

int kinc_absi(int value) {
#ifdef KINC_NO_CLIB
	return value < 0 ? -value : value;
#else
	return abs(value);
#endif
}
