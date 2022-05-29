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

#include <xmmintrin.h>

typedef __m128 kinc_float32x4_t;
typedef __m128 kinc_float32x4_mask_t;

#define KINC_SSE
#if defined(__SSE2__) || _M_IX86_FP == 2
#define KINC_SSE2
typedef __m128i kinc_int8x16_t;
typedef __m128i kinc_int8x16_mask_t;
typedef __m128i kinc_uint8x16_t;
typedef __m128i kinc_uint8x16_mask_t;
typedef __m128i kinc_int16x8_t;
typedef __m128i kinc_int16x8_mask_t;
typedef __m128i kinc_uint16x8_t;
typedef __m128i kinc_uint16x8_mask_t;
typedef __m128i kinc_int32x4_t;
typedef __m128i kinc_int32x4_mask_t;
typedef __m128i kinc_uint32x4_t;
typedef __m128i kinc_uint32x4_mask_t;
#endif

#elif defined(KORE_IOS) || defined(KORE_SWITCH) || defined(__aarch64__) || defined(KORE_NEON)

#define KINC_NEON
#include <arm_neon.h>

typedef float32x4_t kinc_float32x4_t;
typedef uint32x4_t kinc_float32x4_mask_t;
typedef int8x16_t kinc_int8x16_t;
typedef uint8x16_t kinc_int8x16_mask_t;
typedef uint8x16_t kinc_uint8x16_t;
typedef uint8x16_t kinc_uint8x16_mask_t;
typedef int16x8_t kinc_int16x8_t;
typedef uint16x8_t kinc_int16x8_mask_t;
typedef uint16x8_t kinc_uint16x8_t;
typedef uint16x8_t kinc_uint16x8_mask_t;
typedef int32x4_t kinc_int32x4_t;
typedef uint32x4_t kinc_int32x4_mask_t;
typedef uint32x4_t kinc_uint32x4_t;
typedef uint32x4_t kinc_uint32x4_mask_t;

#else

#define KINC_NOSIMD
#include <kinc/math/core.h>

typedef struct kinc_float32x4 {
	float values[4];
} kinc_float32x4_t;

typedef struct kinc_float32x4_mask {
	int values[4];
} kinc_float32x4_mask_t;

#endif

#if (defined(KINC_SSE) && !defined(KINC_SSE2)) || defined(KINC_NOSIMD)

typedef struct kinc_int8x16 {
	int8_t values[16];
} kinc_int8x16_t;

typedef struct kinc_int8x16_mask {
	uint8_t values[16];
} kinc_int8x16_mask_t;

typedef struct kinc_uint8x16 {
	uint8_t values[16];
} kinc_uint8x16_t;

typedef struct kinc_uint8x16_mask {
	uint8_t values[16];
} kinc_uint8x16_mask_t;

typedef struct kinc_int16x8 {
	int16_t values[8];
} kinc_int16x8_t;

typedef struct kinc_int16x8_mask {
	int16_t values[8];
} kinc_int16x8_mask_t;

typedef struct kinc_uint16x8 {
	uint16_t values[8];
} kinc_uint16x8_t;

typedef struct kinc_uint16x8_mask {
	uint16_t values[8];
} kinc_uint16x8_mask_t;

typedef struct kinc_int32x4 {
	int32_t values[4];
} kinc_int32x4_t;

typedef struct kinc_int32x4_mask {
	int32_t values[4];
} kinc_int32x4_mask_t;

typedef struct kinc_uint32x4 {
	uint32_t values[4];
} kinc_uint32x4_t;

typedef struct kinc_uint32x4_mask {
	uint32_t values[4];
} kinc_uint32x4_mask_t;

#endif

#ifdef __cplusplus
}
#endif
