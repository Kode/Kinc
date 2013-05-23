#pragma once

namespace Kore {
	const float pi  = 3.141592654f;
	const float tau = 6.283185307f; // :)
	float sin(float value);
	float cos(float value);
	float tan(float x);
	float cot(float x);
	double round(double value);
	float round(float value);
	int roundUp(float value);
	unsigned pow(unsigned value, unsigned exponent);
	float pow(float value, float exponent);
	double pow(double value, double exponent);
	float maxfloat();
	float sqrt(float value);
	float abs(float value);
	int abs(int value);
	float asin(float value);
	float acos(float value);
	float atan2(float y, float x);
	float floor(float value);

	template<class T> T min(T a, T b) { return (a < b) ? a : b; }
	template<class T> T max(T a, T b) { return (a > b) ? a : b; }
}
