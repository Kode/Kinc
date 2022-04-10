#pragma once

#include <kinc/global.h>

/*! \file types.h
    \brief Provides 128bit SIMD types which are mapped to equivalent SSE or Neon types.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SSE__) || _M_IX86_FP == 2 || _M_IX86_FP == 1 || (defined(KORE_WINDOWS) && !defined(__aarch64__)) ||                                              \
    (defined(KORE_WINDOWSAPP) && !defined(__aarch64__)) || (defined(KORE_MACOS) && __x86_64)

#define KINC_SSE
#if !defined(__SSE2__) && _M_IX86_FP < 2
#define KINC_NOSSE2
#endif

#include <xmmintrin.h>

typedef __m128 kinc_float32x4_t;
typedef __m128 kinc_float32x4_mask_t;


#elif defined(KORE_IOS) || defined(KORE_SWITCH) || defined(__aarch64__) || defined(KORE_NEON)

#define KINC_NEON
#include <arm_neon.h>

typedef float32x4_t kinc_float32x4_t;
typedef uint32x4_t kinc_float32x4_mask_t;

#else

#include <kinc/math/core.h>

typedef struct kinc_float32x4 {
	float values[4];
} kinc_float32x4_t;

typedef struct kinc_float32x4_mask {
	int values[4];
} kinc_float32x4_mask_t;

#endif

#ifdef __cplusplus
}
#endif
