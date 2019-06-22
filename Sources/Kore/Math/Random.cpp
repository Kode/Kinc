#include "pch.h"

#include "Random.h"

#include <kinc/math/random.h>

using namespace Kore;

void Random::init(int seed) {
	kinc_random_init(seed);
}

s32 Random::get() {
	return kinc_random_get();
}

s32 Random::get(s32 max) {
	return kinc_random_get_max(max);
}

s32 Random::get(s32 min, s32 max) {
	return kinc_random_get_in(min, max);
}
