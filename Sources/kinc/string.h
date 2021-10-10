#pragma once

#include <kinc/global.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC size_t kinc_string_length(const char *str);

KINC_FUNC char *kinc_string_copy(char *destination, const char *source);

KINC_FUNC char *kinc_string_copy_limited(char *destination, const char *source, size_t num);

KINC_FUNC char *kinc_string_append(char *destination, const char *source);

KINC_FUNC char *kinc_string_find(char *str1, const char *str2);

KINC_FUNC int kinc_string_compare(const char *str1, const char *str2);

KINC_FUNC int kinc_string_compare_limited(const char *str1, const char *str2, size_t num);

KINC_FUNC size_t kinc_wstring_length(const wchar_t *str);

KINC_FUNC wchar_t *kinc_wstring_copy(wchar_t *destination, const wchar_t *source);

KINC_FUNC wchar_t *kinc_wstring_copy_limited(wchar_t *destination, const wchar_t *source, size_t num);

KINC_FUNC wchar_t *kinc_wstring_append(wchar_t *destination, const wchar_t *source);

KINC_FUNC wchar_t *kinc_wstring_find(wchar_t *str1, const wchar_t *str2);

KINC_FUNC int kinc_wstring_compare(const wchar_t *str1, const wchar_t *str2);

KINC_FUNC int kinc_wstring_compare_limited(const wchar_t *str1, const wchar_t *str2, size_t num);

#ifdef __cplusplus
}
#endif
