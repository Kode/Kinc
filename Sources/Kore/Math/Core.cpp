#include "pch.h"

#include "Core.h"

#include <kinc/math/core.h>

float Kore::tan(float x) {
	return kinc_tan(x);
}

float Kore::cot(float x) {
	return kinc_cot(x);
}

float Kore::round(float value) {
	return kinc_round(value);
}

float Kore::floor(float value) {
	return kinc_floor(value);
}

float Kore::mod(float numer, float denom) {
	return kinc_mod(numer, denom);
}

float Kore::exp(float exponent) {
	return kinc_exp(exponent);
}

float Kore::pow(float value, float exponent) {
	return kinc_pow(value, exponent);
}

float Kore::maxfloat() {
	return kinc_max_float();
}

float Kore::sqrt(float value) {
	return kinc_sqrt(value);
}

float Kore::abs(float value) {
	return kinc_abs(value);
}

float Kore::sin(float value) {
	return kinc_sin(value);
}

float Kore::cos(float value) {
	return kinc_cos(value);
}

float Kore::asin(float value) {
	return kinc_asin(value);
}

float Kore::acos(float value) {
	return kinc_acos(value);
}

float Kore::atan(float value) {
	return kinc_atan(value);
}

float Kore::atan2(float y, float x) {
	return kinc_atan2(y, x);
}

int Kore::roundUp(float value) {
	return static_cast<int>(kinc_ceil(value));
}

unsigned Kore::pow(unsigned value, unsigned exponent) {
	uint result = 1;
	if (!exponent) return result;
	for (;;) {
		if (exponent & 1) result *= value;
		value *= value;
		if (!(exponent >>= 1)) return result;
	}
}

int Kore::abs(int value) {
	return value < 0 ? -value : value;
}

#include <math.h>

double Kore::round(double value) {
	return ::floor(value + 0.5);
}

double Kore::pow(double value, double exponent) {
	return ::pow(value, exponent);
}
