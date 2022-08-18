#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *str);

char *strcpy(char *destination, const char *source);

char *strncpy(char *destination, const char *source, size_t num);

char *strcat(char *destination, const char *source);

char *strstr(const char *str1, const char *str2);

int strcmp(const char *str1, const char *str2);

int strncmp(const char *str1, const char *str2, size_t num);

size_t wcslen(const wchar_t *str);

wchar_t *wcscpy(wchar_t *destination, const wchar_t *source);

wchar_t *wcsncpy(wchar_t *destination, const wchar_t *source, size_t num);

wchar_t *wcscat(wchar_t *destination, const wchar_t *source);

wchar_t *wcsstr(wchar_t *str1, const wchar_t *str2);

int wcscmp(const wchar_t *str1, const wchar_t *str2);

int wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t num);

#ifdef __cplusplus
}
#endif
