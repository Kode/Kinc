#include "string.h"

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
