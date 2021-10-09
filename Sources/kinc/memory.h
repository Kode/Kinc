#pragma once

#include <kinc/global.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void *kinc_allocate(size_t size);

KINC_FUNC void *kinc_reallocate(void *mem, size_t size);

KINC_FUNC void kinc_free(void *mem);

KINC_FUNC void *kinc_memset(void *ptr, int value, size_t num);

KINC_FUNC void *kinc_memcpy(void *destination, const void *source, size_t num);

KINC_FUNC int kinc_memcmp(const void *ptr1, const void *ptr2, size_t num);

#ifdef __cplusplus
}
#endif
