#pragma once

#include <kinc/global.h>

/*! \file core.h
    \brief Just a few very simple additions to math.h
   the C-lib.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

KINC_FUNC float kinc_cot(float x);
KINC_FUNC float kinc_round(float value);
KINC_FUNC float kinc_abs(float value);
KINC_FUNC float kinc_min(float a, float b);
KINC_FUNC float kinc_max(float a, float b);
KINC_FUNC int kinc_mini(int a, int b);
KINC_FUNC int kinc_maxi(int a, int b);
KINC_FUNC float kinc_clamp(float value, float minValue, float maxValue);

#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <float.h>

#include <math.h>
#include <stdlib.h>

float kinc_cot(float x) {
	return cosf(x) / sinf(x);
}

float kinc_round(float value) {
#ifdef KINC_NO_CLIB
	return (float)(int)(value + 0.5f);
#else
	return floorf(value + 0.5f);
#endif
}

float kinc_abs(float value) {
	return value < 0 ? -value : value;
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

#endif

#ifdef __cplusplus
}
#endif
