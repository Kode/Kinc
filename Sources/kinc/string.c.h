#include "string.h"

#include <string.h>
#include <wchar.h>

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
