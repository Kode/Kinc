#pragma once

#include <kinc/global.h>
#include <assert.h>

/*! \file random.h
    \brief Generates values which are kind of random.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Initialize the randomizer.
/// </summary>
/// <param name="seed">A value which should ideally be pretty random</param>
KINC_FUNC void kinc_random_init(int seed);

/// <summary>
/// Returns a random value.
/// </summary>
/// <returns>A random value</returns>
KINC_FUNC int kinc_random_get(void);

/// <summary>
/// Returns a value between 0 and max (both inclusive).
/// </summary>
/// <returns>A random value</returns>
KINC_FUNC int kinc_random_get_max(int max);

/// <summary>
/// Returns a value between min and max (both inclusive).
/// </summary>
/// <returns>A random value</returns>
KINC_FUNC int kinc_random_get_in(int min, int max);

#ifdef KINC_IMPLEMENTATION_MATH
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

// MT19937

static int MT[624];
static int random_initialized = 0;
static int mermeme_index = 0;

static void generateNumbers() {
	for (int i = 0; i < 624; ++i) {
		int y = (MT[i] & 1) + (MT[(i + 1) % 624]) & 0x7fffffff;
		MT[i] = MT[(i + 397) % 624] ^ (y >> 1);
		if ((y % 2) != 0)
			MT[i] = MT[i] ^ 0x9908b0df;
	}
}

void kinc_random_init(int seed) {
	MT[0] = seed;
	for (int i = 1; i < 624; ++i)
		MT[i] = 0x6c078965 * (MT[i - 1] ^ (MT[i - 1] >> 30)) + i;
	random_initialized = 1;
}

int kinc_random_get() {
	assert(random_initialized || !"kinc_random_init() must be called before using kinc_random_get()");

	if (mermeme_index == 0)
		generateNumbers();

	int y = MT[mermeme_index];
	y = y ^ (y >> 11);
	y = y ^ ((y << 7) & (0x9d2c5680));
	y = y ^ ((y << 15) & (0xefc60000));
	y = y ^ (y >> 18);

	mermeme_index = (mermeme_index + 1) % 624;
	return y;
}

int kinc_random_get_max(int max) {
	return kinc_random_get() % (max + 1);
}

int kinc_random_get_in(int min, int max) {
	return kinc_random_get() % (max + 1 - min) + min;
}

#endif

#ifdef __cplusplus
}
#endif
