#include "pch.h"

#include "Core.h"

#include <Kinc/Math/Core.h>

float Kore::tan(float x) {
	return Kinc_Tan(x);
}

float Kore::cot(float x) {
	return Kinc_Cot(x);
}

float Kore::round(float value) {
	return Kinc_Round(value);
}

float Kore::floor(float value) {
	return Kinc_Floor(value);
}

float Kore::mod(float numer, float denom) {
	return Kinc_Mod(numer, denom);
}

float Kore::exp(float exponent) {
	return Kinc_Exp(exponent);
}

float Kore::pow(float value, float exponent) {
	return Kinc_Pow(value, exponent);
}

float Kore::maxfloat() {
	return Kinc_Maxfloat();
}

float Kore::sqrt(float value) {
	return Kinc_Sqrt(value);
}

float Kore::abs(float value) {
	return Kinc_Abs(value);
}

float Kore::sin(float value) {
	return Kinc_Sin(value);
}

float Kore::cos(float value) {
	return Kinc_Cos(value);
}

float Kore::asin(float value) {
	return Kinc_Asin(value);
}

float Kore::acos(float value) {
	return Kinc_Acos(value);
}

float Kore::atan(float value) {
	return Kinc_Atan(value);
}

float Kore::atan2(float y, float x) {
	return Kinc_Atan2(y, x);
}

int Kore::roundUp(float value) {
	return static_cast<int>(Kinc_Ceil(value));
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
