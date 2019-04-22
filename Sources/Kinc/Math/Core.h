#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_PI 3.141592654
#define KINC_TAU 6.283185307

float Kinc_Sin(float value);
float Kinc_Cos(float value);
float Kinc_Tan(float x);
float Kinc_Cot(float x);
float Kinc_Round(float value);
float Kinc_Ceil(float value);
float Kinc_Pow(float value, float exponent);
float Kinc_Maxfloat();
float Kinc_Sqrt(float value);
float Kinc_Abs(float value);
float Kinc_Asin(float value);
float Kinc_Acos(float value);
float Kinc_Atan(float value);
float Kinc_Atan2(float y, float x);
float Kinc_Floor(float value);
float Kinc_Mod(float numer, float denom);
float Kinc_Exp(float exponent);
float Kinc_Min(float a, float b);
float Kinc_Max(float a, float b);
int Kinc_Mini(int a, int b);
int Kinc_Maxi(int a, int b);
float Kinc_Clamp(float value, float minValue, float maxValue);

#ifdef __cplusplus
}
#endif
