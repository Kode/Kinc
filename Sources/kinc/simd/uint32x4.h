#pragma once

#include "types.h"

/*! \file uint32x4.h
    \brief Provides 128bit four-element unsigned 32-bit integer SIMD operations which are mapped to equivalent SSE2 or Neon operations.
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(KINC_SSE2)

static inline kinc_uint32x4_t kinc_uint32x4_intrin_load(const uint32_t *values) {
	return _mm_load_si128((const kinc_uint32x4_t *)values);
}

static inline kinc_uint32x4_t kinc_uint32x4_load(const uint32_t values[4]) {
	return _mm_set_epi32(values[3], values[2], values[1], values[0]);
}

static inline kinc_uint32x4_t kinc_uint32x4_load_all(uint32_t t) {
	return _mm_set1_epi32(t);
}

static inline void kinc_uint32x4_store(uint32_t *destination, kinc_uint32x4_t value) {
	_mm_store_si128((kinc_uint32x4_t *)destination, value);
}

static inline uint32_t kinc_uint32x4_get(kinc_uint32x4_t t, int index) {
	union {
		__m128i value;
		uint32_t elements[4];
	} converter;
	converter.value = t;
	return converter.elements[index];
}

static inline kinc_uint32x4_t kinc_uint32x4_add(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_add_epi32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_sub(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_sub_epi32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_max(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	//No obvious intrinsic here; use a subpar fallback method
	uint32_t values[4];

	for(int i = 0; i < 4; ++i) {

		uint32_t a_single = kinc_uint32x4_get(a, i);
		uint32_t b_single = kinc_uint32x4_get(b, i);

		values[i] = a_single > b_single ? a_single : b_single;
	}

	return kinc_uint32x4_load(values);
}

static inline kinc_uint32x4_t kinc_uint32x4_min(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	//No obvious intrinsic here; use a subpar fallback method
	uint32_t values[4];

	for(int i = 0; i < 4; ++i) {

		uint32_t a_single = kinc_uint32x4_get(a, i);
		uint32_t b_single = kinc_uint32x4_get(b, i);

		values[i] = a_single > b_single ? b_single : a_single;
	}

	return kinc_uint32x4_load(values);
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpeq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_cmpeq_epi32(a, b);
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpneq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_andnot_si128(_mm_cmpeq_epi32(a, b), _mm_set1_epi32(0xffffffff));
}

static inline kinc_uint32x4_t kinc_uint32x4_sel(kinc_uint32x4_t a, kinc_uint32x4_t b, kinc_uint32x4_mask_t mask) {
	return _mm_xor_si128(b, _mm_and_si128(mask, _mm_xor_si128(a, b)));
}

static inline kinc_uint32x4_t kinc_uint32x4_or(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_or_si128(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_and(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_and_si128(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_xor(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return _mm_xor_si128(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_not(kinc_uint32x4_t t) {
	return _mm_xor_si128(t, _mm_set1_epi32(0xffffffff));
}

#elif defined(KINC_NEON)

static inline kinc_uint32x4_t kinc_uint32x4_intrin_load(const uint32_t *values) {
	return vld1q_u32(values);
}

static inline kinc_uint32x4_t kinc_uint32x4_load(const uint32_t values[4]) {
	kinc_uint32x4_t value = vdupq_n_u32(0);
	value = vsetq_lane_u32(values[0], value, 0);
	value = vsetq_lane_u32(values[1], value, 1);
	value = vsetq_lane_u32(values[2], value, 2);
	value = vsetq_lane_u32(values[3], value, 3);

	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_load_all(uint32_t t) {
	return vdupq_n_u32(t);
}

static inline void kinc_uint32x4_store(uint32_t *destination, kinc_uint32x4_t value) {
	vst1q_u32(destination, value);
}

static inline uint32_t kinc_uint32x4_get(kinc_uint32x4_t t, int index) {
	return t[index];
}

static inline kinc_uint32x4_t kinc_uint32x4_add(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vaddq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_sub(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vsubq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_max(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vmaxq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_min(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vminq_u32(a, b);
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpeq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vceqq_u32(a, b);
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpneq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vmvnq_u32(vceqq_u32(a, b));
}

static inline kinc_uint32x4_t kinc_uint32x4_sel(kinc_uint32x4_t a, kinc_uint32x4_t b, kinc_uint32x4_mask_t mask) {
	return vbslq_u32(mask, a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_or(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vorrq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_and(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return vandq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_xor(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	return veorq_u32(a, b);
}

static inline kinc_uint32x4_t kinc_uint32x4_not(kinc_uint32x4_t t) {
	return vmvnq_u32(t);
}

#else

static inline kinc_uint32x4_t kinc_uint32x4_intrin_load(const uint32_t *values) {
	kinc_uint32x4_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_load(const uint32_t values[4]) {
	kinc_uint32x4_t value;
	value.values[0] = values[0];
	value.values[1] = values[1];
	value.values[2] = values[2];
	value.values[3] = values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_load_all(uint32_t t) {
	kinc_uint32x4_t value;
	value.values[0] = t;
	value.values[1] = t;
	value.values[2] = t;
	value.values[3] = t;
	return value;
}

static inline void kinc_uint32x4_store(uint32_t *destination, kinc_uint32x4_t value) {
	destination[0] = value.values[0];
	destination[1] = value.values[1];
	destination[2] = value.values[2];
	destination[3] = value.values[3];
}

static inline uint32_t kinc_uint32x4_get(kinc_uint32x4_t t, int index) {
	return t.values[index];
}

static inline kinc_uint32x4_t kinc_uint32x4_add(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] + b.values[0];
	value.values[1] = a.values[1] + b.values[1];
	value.values[2] = a.values[2] + b.values[2];
	value.values[3] = a.values[3] + b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_sub(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] - b.values[0];
	value.values[1] = a.values[1] - b.values[1];
	value.values[2] = a.values[2] - b.values[2];
	value.values[3] = a.values[3] - b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_max(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] > b.values[0] ? a.values[0] : b.values[0];
	value.values[1] = a.values[1] > b.values[1] ? a.values[1] : b.values[1]; 
	value.values[2] = a.values[2] > b.values[2] ? a.values[2] : b.values[2]; 
	value.values[3] = a.values[3] > b.values[3] ? a.values[3] : b.values[3]; 
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_min(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] > b.values[0] ? b.values[0] : a.values[0];
	value.values[1] = a.values[1] > b.values[1] ? b.values[1] : a.values[1]; 
	value.values[2] = a.values[2] > b.values[2] ? b.values[2] : a.values[2]; 
	value.values[3] = a.values[3] > b.values[3] ? b.values[3] : a.values[3]; 
	return value;
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpeq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_mask_t mask;
	mask.values[0] = a.values[0] == b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] == b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] == b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] == b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_uint32x4_mask_t kinc_uint32x4_cmpneq(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_mask_t mask;
	mask.values[0] = a.values[0] != b.values[0] ? 0xffffffff : 0;
	mask.values[1] = a.values[1] != b.values[1] ? 0xffffffff : 0;
	mask.values[2] = a.values[2] != b.values[2] ? 0xffffffff : 0;
	mask.values[3] = a.values[3] != b.values[3] ? 0xffffffff : 0;
	return mask;
}

static inline kinc_uint32x4_t kinc_uint32x4_sel(kinc_uint32x4_t a, kinc_uint32x4_t b, kinc_uint32x4_mask_t mask) {
	kinc_uint32x4_t value;
	value.values[0] = mask.values[0] != 0 ? a.values[0] : b.values[0];
	value.values[1] = mask.values[1] != 0 ? a.values[1] : b.values[1];
	value.values[2] = mask.values[2] != 0 ? a.values[2] : b.values[2];
	value.values[3] = mask.values[3] != 0 ? a.values[3] : b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_or(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] | b.values[0];
	value.values[1] = a.values[1] | b.values[1];
	value.values[2] = a.values[2] | b.values[2];
	value.values[3] = a.values[3] | b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_and(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] & b.values[0];
	value.values[1] = a.values[1] & b.values[1];
	value.values[2] = a.values[2] & b.values[2];
	value.values[3] = a.values[3] & b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_xor(kinc_uint32x4_t a, kinc_uint32x4_t b) {
	kinc_uint32x4_t value;
	value.values[0] = a.values[0] ^ b.values[0];
	value.values[1] = a.values[1] ^ b.values[1];
	value.values[2] = a.values[2] ^ b.values[2];
	value.values[3] = a.values[3] ^ b.values[3];
	return value;
}

static inline kinc_uint32x4_t kinc_uint32x4_not(kinc_uint32x4_t t) {
	kinc_uint32x4_t value;
	value.values[0] = ~t.values[0];
	value.values[1] = ~t.values[1];
	value.values[2] = ~t.values[2];
	value.values[3] = ~t.values[3];
	return value;
}

#endif

#ifdef __cplusplus
}
#endif
