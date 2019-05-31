#include "pch.h"

#include "Random.h"

#include <kinc/math/random.h>

using namespace Kore;

void Random::init(int seed) {
	Kinc_Random_Init(seed);
}

s32 Random::get() {
	return Kinc_Random_Get();
}

s32 Random::get(s32 max) {
	return Kinc_Random_GetMax(max);
}

s32 Random::get(s32 min, s32 max) {
	return Kinc_Random_GetIn(min, max);
}
