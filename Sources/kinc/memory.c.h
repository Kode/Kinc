#include "memory.h"

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
