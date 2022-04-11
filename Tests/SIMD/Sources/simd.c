#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/simd/float32x4.h>
#include <kinc/simd/int8x16.h>
#include <kinc/system.h>
#include <stdbool.h>

#define EPSILON 0.00001f

static int total_tests = 0;

static bool check_f32(const char *name, kinc_float32x4_t result, const float expected[4], float epsilon) {
	++total_tests;
	bool success = true;
	for (int i = 0; i < 4; ++i) {
		if (kinc_abs(kinc_float32x4_get(result, i) - expected[i]) > epsilon) {
			success = false;
		}
	}
	kinc_log(KINC_LOG_LEVEL_ERROR, "Test %s %s", name, success ? "PASS" : "FAIL");
	if (!success) {
		kinc_log(KINC_LOG_LEVEL_INFO, "\texpected {%f, %f, %f, %f} got {%f, %f, %f, %f}", expected[0], expected[1], expected[2], expected[3],
		         kinc_float32x4_get(result, 0), kinc_float32x4_get(result, 1), kinc_float32x4_get(result, 2), kinc_float32x4_get(result, 3));
	}
	return success;
}

static bool check_i8(const char *name, kinc_int8x16_t result, const int8_t expected[16]) {
	++total_tests;
	bool success = true;
	for (int i = 0; i < 16; ++i) {
		if (kinc_int8x16_get(result, i) != expected[i]) {
			success = false;
		}
	}
	kinc_log(KINC_LOG_LEVEL_ERROR, "Test %s %s", name, success ? "PASS" : "FAIL");
	if (!success) {
		kinc_log(
		    KINC_LOG_LEVEL_INFO,
		    "\texpected {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d} got {%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d}",
		    expected[0], expected[1], expected[2], expected[3], expected[4], expected[5], expected[6], expected[7], expected[8], expected[9], expected[10],
		    expected[11], expected[12], expected[13], expected[14], expected[15], kinc_int8x16_get(result, 0), kinc_int8x16_get(result, 1),
		    kinc_int8x16_get(result, 2), kinc_int8x16_get(result, 3), kinc_int8x16_get(result, 4), kinc_int8x16_get(result, 5), kinc_int8x16_get(result, 6),
		    kinc_int8x16_get(result, 7), kinc_int8x16_get(result, 8), kinc_int8x16_get(result, 9), kinc_int8x16_get(result, 10), kinc_int8x16_get(result, 11),
		    kinc_int8x16_get(result, 12), kinc_int8x16_get(result, 13), kinc_int8x16_get(result, 14), kinc_int8x16_get(result, 15));
	}
	return success;
}

int kickstart(int argc, char **argv) {
#if defined(KINC_SSE)
	kinc_log(KINC_LOG_LEVEL_INFO, "Using SSE\n");
#elif defined(KINC_NEON)
	kinc_log(KINC_LOG_LEVEL_INFO, "Using NEON\n");
#else
	kinc_log(KINC_LOG_LEVEL_INFO, "Using scalar fallback implementation\n");
#endif

	int failed = 0;
	{
		kinc_float32x4_t a = kinc_float32x4_load(-1.0f, 2.0f, 3.0f, 4.0f);
		kinc_float32x4_t b = kinc_float32x4_load_all(2.0f);

		kinc_float32x4_mask_t mask;
		kinc_float32x4_t result;

		result = kinc_float32x4_abs(a);
		failed += check_f32("float32x4 abs", result, (float[4]){1.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_add(a, b);
		failed += check_f32("float32x4 add", result, (float[4]){1.0f, 4.0f, 5.0f, 6.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_sub(a, b);
		failed += check_f32("float32x4 sub", result, (float[4]){-3.0f, 0.0f, 1.0f, 2.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_mul(a, b);
		failed += check_f32("float32x4 mul", result, (float[4]){-2.0f, 4.0f, 6.0f, 8.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_div(a, b);
		failed += check_f32("float32x4 div", result, (float[4]){-0.5f, 1.0f, 1.5f, 2.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_neg(a);
		failed += check_f32("float32x4 neg", result, (float[4]){1.0f, -2.0f, -3.0f, -4.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_reciprocal_approximation(a);
		failed += check_f32("float32x4 reciprocal_approximation", result, (float[4]){1.0f / -1.0f, 1.0f / 2.0f, 1.0f / 3.0f, 1.0f / 4.0f}, 0.002f) ? 0 : 1;

		result = kinc_float32x4_reciprocal_sqrt_approximation(b);
		failed += check_f32("float32x4 reciprocal_sqrt_approximation", result,
		                    (float[4]){1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f)}, 0.003f)
		              ? failed
		              : 1;

		result = kinc_float32x4_sqrt(b);
		failed += check_f32("float32x4 sqrt", result, (float[4]){kinc_sqrt(2.0f), kinc_sqrt(2.0f), kinc_sqrt(2.0f), kinc_sqrt(2.0f)}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_max(a, b);
		failed += check_f32("float32x4 max", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

		result = kinc_float32x4_min(a, b);
		failed += check_f32("float32x4 min", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmpeq(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmpeq & sel", result, (float[4]){2.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmpge(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmpge & sel", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmpgt(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmpgt & sel", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmple(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmple & sel", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmplt(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmplt & sel", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

		mask = kinc_float32x4_cmpneq(a, b);
		result = kinc_float32x4_sel(a, b, mask);
		failed += check_f32("float32x4 cmpneq & sel", result, (float[4]){-1.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;
	}

	{
		kinc_int8x16_t a = kinc_int8x16_load((int8_t[16]){-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6, 7, 8});
		kinc_int8x16_t b = kinc_int8x16_load_all(2);

		kinc_int8x16_mask_t mask;
		kinc_int8x16_t result;

		result = kinc_int8x16_add(a, b);
		failed += check_i8("int8x16 add", result, (int8_t[16]){-6, -5, -4, -3, -2, -1, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10}) ? 0 : 1;

		result = kinc_int8x16_sub(a, b);
		failed += check_i8("int8x16 sub", result, (int8_t[16]){-10, -9, -8, -7, -6, -5, -4, -3, -1, 0, 1, 2, 3, 4, 5, 6}) ? 0 : 1;

		result = kinc_int8x16_max(a, b);
		failed += check_i8("int8x16 max", result, (int8_t[16]){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8}) ? 0 : 1;

		result = kinc_int8x16_min(a, b);
		failed += check_i8("int8x16 min", result, (int8_t[16]){-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 2, 2, 2, 2, 2, 2}) ? 0 : 1;

		mask = kinc_int8x16_cmpeq(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmpeq & sel", result, (int8_t[16]){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}) ? 0 : 1;

		mask = kinc_int8x16_cmpge(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmpge & sel", result, (int8_t[16]){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8}) ? 0 : 1;

		mask = kinc_int8x16_cmpgt(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmpgt & sel", result, (int8_t[16]){2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8}) ? 0 : 1;

		mask = kinc_int8x16_cmple(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmple & sel", result, (int8_t[16]){-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 2, 2, 2, 2, 2, 2}) ? 0 : 1;

		mask = kinc_int8x16_cmplt(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmplt & sel", result, (int8_t[16]){-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 2, 2, 2, 2, 2, 2}) ? 0 : 1;

		mask = kinc_int8x16_cmpneq(a, b);
		result = kinc_int8x16_sel(a, b, mask);
		failed += check_i8("int8x16 cmpneq & sel", result, (int8_t[16]){-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6, 7, 8}) ? 0 : 1;

		result = kinc_int8x16_or(a, b);
		failed += check_i8("int8x16 or", result, (int8_t[16]){-8 | 2, -7 | 2, -6 | 2, -5 | 2, -4 | 2, -3 | 2, -2 | 2, -1 | 2, 1 | 2, 2 | 2, 3 | 2, 4 | 2, 5 | 2, 6 | 2, 7 | 2, 8 | 2}) ? 0 : 1;

		result = kinc_int8x16_and(a, b);
		failed += check_i8("int8x16 and", result, (int8_t[16]){-8 & 2, -7 & 2, -6 & 2, -5 & 2, -4 & 2, -3 & 2, -2 & 2, -1 & 2, 1 & 2, 2 & 2, 3 & 2, 4 & 2, 5 & 2, 6 & 2, 7 & 2, 8 & 2}) ? 0 : 1;

		result = kinc_int8x16_xor(a, b);
		failed += check_i8("int8x16 xor", result, (int8_t[16]){-8 ^ 2, -7 ^ 2, -6 ^ 2, -5 ^ 2, -4 ^ 2, -3 ^ 2, -2 ^ 2, -1 ^ 2, 1 ^ 2, 2 ^ 2, 3 ^ 2, 4 ^ 2, 5 ^ 2, 6 ^ 2, 7 ^ 2, 8 ^ 2}) ? 0 : 1;

		result = kinc_int8x16_not(a);
		failed += check_i8("int8x16 not", result, (int8_t[16]){~-8, ~-7, ~-6, ~-5, ~-4, ~-3, ~-2, ~-1, ~1, ~2, ~3, ~4, ~5, ~6, ~7, ~8}) ? 0 : 1;
	}

	if (failed) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "\nERROR! %d of %d test(s) failed", failed, total_tests);
	}
	else {
		kinc_log(KINC_LOG_LEVEL_INFO, "\nSUCCESS %d tests passed", total_tests);
	}

	return failed;
}
