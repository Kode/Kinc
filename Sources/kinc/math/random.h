#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void kinc_random_init(int seed);
int kinc_random_get();
int kinc_random_get_max(int max);
int kinc_random_get_in(int min, int max);

#ifdef __cplusplus
}
#endif
