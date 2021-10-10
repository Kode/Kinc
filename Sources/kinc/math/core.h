#pragma once

#include <kinc/global.h>

/*! \file core.h
    \brief Provides some basic math functions. This is mostly just functionality that also exists in math.h but hey, maybe we can one day run programs without
   the C-lib.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

KINC_FUNC float kinc_sin(float value);
KINC_FUNC float kinc_cos(float value);
KINC_FUNC float kinc_tan(float x);
KINC_FUNC float kinc_cot(float x);
KINC_FUNC float kinc_round(float value);
KINC_FUNC float kinc_ceil(float value);
KINC_FUNC float kinc_pow(float value, float exponent);
KINC_FUNC float kinc_max_float(void);
KINC_FUNC float kinc_sqrt(float value);
KINC_FUNC float kinc_abs(float value);
KINC_FUNC float kinc_asin(float value);
KINC_FUNC float kinc_acos(float value);
KINC_FUNC float kinc_atan(float value);
KINC_FUNC float kinc_atan2(float y, float x);
KINC_FUNC float kinc_floor(float value);
KINC_FUNC float kinc_mod(float numer, float denom);
KINC_FUNC float kinc_exp(float exponent);
KINC_FUNC float kinc_min(float a, float b);
KINC_FUNC float kinc_max(float a, float b);
KINC_FUNC int kinc_mini(int a, int b);
KINC_FUNC int kinc_maxi(int a, int b);
KINC_FUNC float kinc_clamp(float value, float minValue, float maxValue);
KINC_FUNC int kinc_absi(int value);

#ifdef __cplusplus
}
#endif
