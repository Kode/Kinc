#include "pch.h"
#include "Random.h"

using namespace Kore;

//MT19937

namespace {
	s32 MT[624];
	int index = 0;

	void generateNumbers() {
		for (int i = 0; i < 624; ++i) {
			int y = (MT[i] & 1) + (MT[(i + 1) % 624]) & 0x7fffffff;
			MT[i] = MT[(i + 397) % 624] ^ (y >> 1);
			if ((y % 2) != 0) MT[i] = MT[i] ^ 0x9908b0df;
		}
	}
}

void Random::init(int seed) {
	MT[0] = seed;
	for (int i = 1; i < 624; ++i) MT[i] = 0x6c078965 * (MT[i - 1] ^ (MT[i - 1] >> 30)) + i;
}

s32 Random::get() {
	if (index == 0) generateNumbers();

	s32 y = MT[index];
	y = y ^ (y >> 11);
	y = y ^ ((y << 7) & (0x9d2c5680));
	y = y ^ ((y << 15) & (0xefc60000));
	y = y ^ (y >> 18);

	index = (index + 1) % 624;
	return y;
}

s32 Random::get(s32 max) {
	return Random::get() % (max + 1);
}

s32 Random::get(s32 min, s32 max) {
	return Random::get() % (max + 1 - min) + min;
}