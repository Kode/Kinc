#pragma once

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void kinc_random_init(int seed);
KINC_FUNC int kinc_random_get();
KINC_FUNC int kinc_random_get_max(int max);
KINC_FUNC int kinc_random_get_in(int min, int max);

#ifdef __cplusplus
}
#endif
