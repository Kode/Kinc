#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/simd/float32x4.h>
#include <kinc/system.h>
#include <stdbool.h>

#define EPSILON 0.00001f

static bool check(const char *name, kinc_float32x4_t result, const float expected[4], float epsilon) {
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

int kickstart(int argc, char **argv) {
#if defined(KINC_SSE)
    kinc_log(KINC_LOG_LEVEL_INFO, "Using SSE\n");
#elif defined(KINC_NEON)
    kinc_log(KINC_LOG_LEVEL_INFO, "Using NEON\n");
#else
    kinc_log(KINC_LOG_LEVEL_INFO, "Using scalar fallback implementation\n");
#endif

	kinc_float32x4_t a = kinc_float32x4_load(-1.0f, 2.0f, 3.0f, 4.0f);
	kinc_float32x4_t b = kinc_float32x4_load_all(2.0f);

	kinc_float32x4_mask_t mask;
	kinc_float32x4_t result;
	int failed = 0;
	bool cres;

	result = kinc_float32x4_abs(a);
	failed += check("float32x4 abs", result, (float[4]){1.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_add(a, b);
	failed += check("float32x4 add", result, (float[4]){1.0f, 4.0f, 5.0f, 6.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_sub(a, b);
	failed += check("float32x4 sub", result, (float[4]){-3.0f, 0.0f, 1.0f, 2.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_mul(a, b);
	failed += check("float32x4 mul", result, (float[4]){-2.0f, 4.0f, 6.0f, 8.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_div(a, b);
	failed += check("float32x4 div", result, (float[4]){-0.5f, 1.0f, 1.5f, 2.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_neg(a);
	failed += check("float32x4 neg", result, (float[4]){1.0f, -2.0f, -3.0f, -4.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_reciprocal_approximation(a);
	failed += check("float32x4 reciprocal_approximation", result, (float[4]){1.0f / -1.0f, 1.0f / 2.0f, 1.0f / 3.0f, 1.0f / 4.0f}, 0.002f) ? 0 : 1;

	result = kinc_float32x4_reciprocal_sqrt_approximation(b);
	failed += check("float32x4 reciprocal_sqrt_approximation", result,
	                (float[4]){1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f), 1.0f / kinc_sqrt(2.0f)}, 0.003f)
	              ? failed
	              : 1;

	result = kinc_float32x4_sqrt(b);
	failed += check("float32x4 sqrt", result, (float[4]){kinc_sqrt(2.0f), kinc_sqrt(2.0f), kinc_sqrt(2.0f), kinc_sqrt(2.0f)}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_max(a, b);
	failed += check("float32x4 max", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

	result = kinc_float32x4_min(a, b);
	failed += check("float32x4 min", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmpeq(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmpeq & sel", result, (float[4]){2.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmpge(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmpge & sel", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmpgt(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmpgt & sel", result, (float[4]){2.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmple(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmple & sel", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmplt(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmplt & sel", result, (float[4]){-1.0f, 2.0f, 2.0f, 2.0f}, EPSILON) ? 0 : 1;

	mask = kinc_float32x4_cmpneq(a, b);
	result = kinc_float32x4_sel(a, b, mask);
	failed += check("float32x4 cmpneq & sel", result, (float[4]){-1.0f, 2.0f, 3.0f, 4.0f}, EPSILON) ? 0 : 1;

	if (failed) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "\nERROR! %d test(s) failed", failed);
	}
	else {
		kinc_log(KINC_LOG_LEVEL_INFO, "\nSUCCESS all tests passed");
	}

	return failed;
}
