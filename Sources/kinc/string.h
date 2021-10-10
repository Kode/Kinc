#pragma once

#include <kinc/global.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC size_t kinc_string_length(const char *str);

KINC_FUNC char *kinc_string_copy(char *destination, const char *source);

#ifdef __cplusplus
}
#endif
