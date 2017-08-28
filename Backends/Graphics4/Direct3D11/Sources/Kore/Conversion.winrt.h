#pragma once
#include "pch.h"
#include <Kore/Graphics4/Graphics.h>


using namespace Windows::Foundation::Numerics;

inline Kore::mat4 WindowsNumericsToKoreMat(float4x4 m)
{
	Kore::mat4 mat;
	mat.Set(0, 0, m.m11);
	mat.Set(0, 1, m.m21);
	mat.Set(0, 2, m.m31);
	mat.Set(0, 3, m.m41);

	mat.Set(1, 0, m.m12);
	mat.Set(1, 1, m.m22);
	mat.Set(1, 2, m.m32);
	mat.Set(1, 3, m.m42);

	mat.Set(2, 0, m.m13);
	mat.Set(2, 1, m.m23);
	mat.Set(2, 2, m.m33);
	mat.Set(2, 3, m.m43);

	mat.Set(3, 0, m.m14);
	mat.Set(3, 1, m.m24);
	mat.Set(3, 2, m.m34);
	mat.Set(3, 3, m.m44);
	return mat;
}
