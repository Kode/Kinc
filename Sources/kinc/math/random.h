#pragma once

#include <kinc/global.h>

/*! \file random.h
    \brief Generates values which are kind of random.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Initializes the randomizer with a seed. This is optional but helpful.
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

// xoshiro256** 1.0

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t s[4] = {1, 2, 3, 4};

uint64_t next(void) {
	const uint64_t result = rotl(s[1] * 5, 7) * 9;

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}

void kinc_random_init(int seed) {
	s[0] = seed;
	s[1] = next();
	s[2] = next();
	s[3] = next();
}

int kinc_random_get(void) {
	uint64_t value = next();
	return *(int *)&value;
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
