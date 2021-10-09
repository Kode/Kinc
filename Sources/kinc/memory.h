#pragma once

#include <kinc/global.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC void *kinc_allocate(size_t size);

KINC_FUNC void *kinc_reallocate(void *mem, size_t size);

KINC_FUNC void kinc_free(void *mem);

#ifdef __cplusplus
}
#endif
