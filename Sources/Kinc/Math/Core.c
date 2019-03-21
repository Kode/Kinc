#include "pch.h"

#include "Core.h"

#include <float.h>
#include <math.h>

float Kinc_Tan(float x) {
	return tanf(x);
}

float Kinc_Cot(float x) {
	return cosf(x) / sinf(x);
}

float Kinc_Round(float value) {
	return floorf(value + 0.5f);
}

float Kinc_Ceil(float value) {
	return ceilf(value);
}

float Kinc_Floor(float value) {
	return floorf(value);
}

float Kinc_Mod(float numer, float denom) {
	return fmodf(numer, denom);
}

float Kinc_Exp(float exponent) {
	return expf(exponent);
}

float Kinc_Pow(float value, float exponent) {
	return powf(value, exponent);
}

float Kinc_Maxfloat() {
	return FLT_MAX;
}

float Kinc_Sqrt(float value) {
	return sqrtf(value);
}

float Kinc_Abs(float value) {
	return value < 0 ? -value : value;
}

float Kinc_Sin(float value) {
	return sinf(value);
}

float Kinc_Cos(float value) {
	return cosf(value);
}

float Kinc_Asin(float value) {
	return asinf(value);
}

float Kinc_Acos(float value) {
	return acosf(value);
}

float Kinc_Atan(float value) {
	return atanf(value);
}

float Kinc_Atan2(float y, float x) {
	return atan2f(y, x);
}
