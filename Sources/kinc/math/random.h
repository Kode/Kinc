#pragma once

#include <kinc/global.h>

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

#ifdef __cplusplus
}
#endif
