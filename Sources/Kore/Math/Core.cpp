#include "pch.h"
#include "Core.h"

#ifdef SYS_HTML5
typedef float float_t;
typedef double double_t;
#endif

#include <cfloat>
#include <cmath>

using namespace Kore;

float Kore::tan(float x) {
	return std::sin(x) / std::cos(x);
	//return tanf(x);
	// Verhält sich tanf(x) genauso wie sinf(x) / cosf(x) ?
}

float Kore::cot(float x) {
	return std::cos(x) / std::sin(x);
}

double Kore::round(double value) {
	//if (value - floor(value) >= 0.5) return ceil(value);
	//else return floor(value);
	return std::floor(value + 0.5);
}

float Kore::round(float value) {
//	if (value - floorf(value) >= 0.5f) return ceilf(value);
//	else return floorf(value);
	return std::floor(value + 0.5f);
}

int Kore::roundUp(float value) {
	return static_cast<int>(std::ceil(value));
}

float Kore::floor(float value) {
	return std::floor(value);
}

unsigned Kore::pow(unsigned value, unsigned exponent) {
	/*unsigned ret = 1;
	for (unsigned i = 0; i < exponent; ++i) ret *= value;
	return ret;*/

	uint result = 1;
	if (!exponent) return result;
	for (;;) {
		if (exponent & 1) result *= value;
		value *= value;
		if (!(exponent >>= 1)) return result;
	}
}

float Kore::pow(float value, float exponent) {
	return std::pow(value, exponent);
}

double Kore::pow(double value, double exponent) {
	return std::pow(value, exponent);
}

float Kore::maxfloat() {
	return FLT_MAX;
}

float Kore::sqrt(float value) {
	return std::sqrt(value);
}

float Kore::abs(float value) {
	//return ::abs(value);
	return value < 0 ? -value : value;
	// TODO: int-Trick
}

int Kore::abs(int value) {
	return value < 0 ? -value : value;
}

float Kore::sin(float value) {
	return std::sin(value);
}

float Kore::cos(float value) {
	return std::cos(value);
}

float Kore::asin(float value) {
	return std::asin(value);
}

float Kore::acos(float value) {
	return std::acos(value);
}

float Kore::atan2(float y, float x) {
	return std::atan2(y, x);
}
