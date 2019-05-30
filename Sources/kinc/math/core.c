#include "pch.h"

#include "Core.h"

#include <float.h>
#include <math.h>

float kinc_tan(float x) {
	return tanf(x);
}

float kinc_cot(float x) {
	return cosf(x) / sinf(x);
}

float kinc_round(float value) {
	return floorf(value + 0.5f);
}

float kinc_ceil(float value) {
	return ceilf(value);
}

float kinc_floor(float value) {
	return floorf(value);
}

float kinc_mod(float numer, float denom) {
	return fmodf(numer, denom);
}

float kinc_exp(float exponent) {
	return expf(exponent);
}

float kinc_pow(float value, float exponent) {
	return powf(value, exponent);
}

float kinc_max_float() {
	return FLT_MAX;
}

float kinc_sqrt(float value) {
	return sqrtf(value);
}

float kinc_abs(float value) {
	return value < 0 ? -value : value;
}

float kinc_sin(float value) {
	return sinf(value);
}

float kinc_cos(float value) {
	return cosf(value);
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
