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

KINC_FUNC char *kinc_string_duplicate(const char *str);

KINC_FUNC size_t kinc_wstring_length(const wchar_t *str);

KINC_FUNC wchar_t *kinc_wstring_copy(wchar_t *destination, const wchar_t *source);

KINC_FUNC wchar_t *kinc_wstring_copy_limited(wchar_t *destination, const wchar_t *source, size_t num);

KINC_FUNC wchar_t *kinc_wstring_append(wchar_t *destination, const wchar_t *source);

KINC_FUNC wchar_t *kinc_wstring_find(wchar_t *str1, const wchar_t *str2);

KINC_FUNC int kinc_wstring_compare(const wchar_t *str1, const wchar_t *str2);

KINC_FUNC int kinc_wstring_compare_limited(const wchar_t *str1, const wchar_t *str2, size_t num);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <kinc/memory.h>

#include <string.h>
#include <wchar.h>

#define STB_SPRINTF_IMPLEMENTATION
#include <kinc/libs/stb_sprintf.h>

#undef strcmp
#undef strncmp

size_t kinc_string_length(const char *str) {
#ifdef KINC_NO_CLIB
	size_t size = 0;
	while (true) {
		if (str[size] == 0) {
			return size;
		}
		++size;
	}
	return 0;
#else
	return strlen(str);
#endif
}

char *kinc_string_copy(char *destination, const char *source) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0;; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
#else
	return strcpy(destination, source);
#endif
}

char *kinc_string_copy_limited(char *destination, const char *source, size_t num) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0; i < num; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
#else
	return strncpy(destination, source, num);
#endif
}

char *kinc_string_append(char *destination, const char *source) {
#ifdef KINC_NO_CLIB
	size_t di = 0;
	while (destination[di] != 0) {
		++di;
	}
	for (size_t si = 0;; ++si) {
		destination[di] = source[si];
		if (source[si] == 0) {
			return destination;
		}
		++di;
	}
	return destination;
#else
	return strcat(destination, source);
#endif
}

char *kinc_string_find(char *str1, const char *str2) {
#ifdef KINC_NO_CLIB
	for (size_t i1 = 0;; ++i1) {
		if (str1[i1] == 0) {
			return NULL;
		}
		for (size_t i2 = 0;; ++i2) {
			if (str2[i2] == 0) {
				return &str1[i1];
			}
			if (str1[i1 + i2] != str2[i2]) {
				break;
			}
		}
	}
#else
	return strstr(str1, str2);
#endif
}

int kinc_string_compare(const char *str1, const char *str2) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0;; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
#else
	return strcmp(str1, str2);
#endif
}

int kinc_string_compare_limited(const char *str1, const char *str2, size_t num) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0; i < num; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
	return 0;
#else
	return strncmp(str1, str2, num);
#endif
}

char *kinc_string_duplicate(const char *str) {
	char *ret = kinc_allocate(kinc_string_length(str) + 1);
	kinc_string_copy(ret, str);
	return ret;
}

size_t kinc_wstring_length(const wchar_t *str) {
#ifdef KINC_NO_CLIB
	size_t size = 0;
	while (true) {
		if (str[size] == 0) {
			return size;
		}
		++size;
	}
	return 0;
#else
	return wcslen(str);
#endif
}

wchar_t *kinc_wstring_copy(wchar_t *destination, const wchar_t *source) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0;; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
#else
	return wcscpy(destination, source);
#endif
}

wchar_t *kinc_wstring_copy_limited(wchar_t *destination, const wchar_t *source, size_t num) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0; i < num; ++i) {
		destination[i] = source[i];
		if (source[i] == 0) {
			return destination;
		}
	}
	return destination;
#else
	return wcsncpy(destination, source, num);
#endif
}

wchar_t *kinc_wstring_append(wchar_t *destination, const wchar_t *source) {
#ifdef KINC_NO_CLIB
	size_t di = 0;
	while (destination[di] != 0) {
		++di;
	}
	for (size_t si = 0;; ++si) {
		destination[di] = source[si];
		if (source[si] == 0) {
			return destination;
		}
		++di;
	}
	return destination;
#else
	return wcscat(destination, source);
#endif
}

wchar_t *kinc_wstring_find(wchar_t *str1, const wchar_t *str2) {
#ifdef KINC_NO_CLIB
	for (size_t i1 = 0;; ++i1) {
		if (str1[i1] == 0) {
			return NULL;
		}
		for (size_t i2 = 0;; ++i2) {
			if (str2[i2] == 0) {
				return &str1[i1];
			}
			if (str1[i1 + i2] != str2[i2]) {
				break;
			}
		}
	}
#else
	return wcsstr(str1, str2);
#endif
}

int kinc_wstring_compare(const wchar_t *str1, const wchar_t *str2) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0;; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
#else
	return wcscmp(str1, str2);
#endif
}

int kinc_wstring_compare_limited(const wchar_t *str1, const wchar_t *str2, size_t num) {
#ifdef KINC_NO_CLIB
	for (size_t i = 0; i < num; ++i) {
		if (str1[i] != str2[i]) {
			return str1[i] - str2[i];
		}
		if (str1[i] == 0) {
			return 0;
		}
	}
	return 0;
#else
	return wcsncmp(str1, str2, num);
#endif
}

#endif

#ifdef __cplusplus
}
#endif
