#include "memory.h"

#undef memcpy
#undef memset

void *kinc_allocate(size_t size) {
#ifdef KINC_NO_CLIB
	return NULL;
#else
	return malloc(size);
#endif
}

void *kinc_reallocate(void *mem, size_t size) {
#ifdef KINC_NO_CLIB
	return NULL;
#else
	return realloc(mem, size);
#endif
}

void kinc_free(void *mem) {
#ifdef KINC_NO_CLIB

#else
	free(mem);
#endif
}

KINC_FUNC void *kinc_memset(void *ptr, int value, size_t num) {
#ifdef KINC_NO_CLIB
	unsigned char *data = (unsigned char *)ptr;
	for (size_t i = 0; i < num; ++i) {
		data[i] = (unsigned char)value;
	}
	return ptr;
#else
	return memset(ptr, value, num);
#endif
}

KINC_FUNC void *kinc_memcpy(void *destination, const void *source, size_t num) {
#ifdef KINC_NO_CLIB
	unsigned char *s = (unsigned char *)source;
	unsigned char *d = (unsigned char *)destination;
	for (size_t i = 0; i < num; ++i) {
		d[i] = s[i];
	}
	return destination;
#else
	return memcpy(destination, source, num);
#endif
}

KINC_FUNC int kinc_memcmp(const void *ptr1, const void *ptr2, size_t num) {
#ifdef KINC_NO_CLIB
	unsigned char *p1 = (unsigned char *)ptr1;
	unsigned char *p2 = (unsigned char *)ptr2;
	for (size_t i = 0; i < num; ++i) {
		if (p1[i] != p2[i]) {
			return (int)p1[i] - (int)p2[i];
		}
	}
	return 0;
#else
	return memcmp(ptr1, ptr2, num);
#endif
}
