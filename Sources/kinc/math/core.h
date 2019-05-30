#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

float kinc_sin(float value);
float kinc_cos(float value);
float kinc_tan(float x);
float kinc_cot(float x);
float kinc_round(float value);
float kinc_ceil(float value);
float kinc_pow(float value, float exponent);
float kinc_max_float();
float kinc_sqrt(float value);
float kinc_abs(float value);
float kinc_asin(float value);
float kinc_acos(float value);
float kinc_atan(float value);
float kinc_atan2(float y, float x);
float kinc_floor(float value);
float kinc_mod(float numer, float denom);
float kinc_exp(float exponent);
float kinc_min(float a, float b);
float kinc_max(float a, float b);
int kinc_mini(int a, int b);
int kinc_maxi(int a, int b);
float kinc_clamp(float value, float minValue, float maxValue);

#ifdef __cplusplus
}
#endif
