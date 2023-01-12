#include "stdlib.h"

#ifdef KORE_WASM
static unsigned char buffer[1024 * 1024];
static unsigned int allocated = 1;
#endif

void *malloc(size_t size) {
#ifdef KORE_WASM
	void *p = &buffer[allocated];
	allocated += size;
	return p;
#endif
	return NULL;
}

void *alloca(size_t size) {
	return NULL;
}

void *realloc(void *mem, size_t size) {
	return NULL;
}

void free(void *mem) {

}

void *memset(void *ptr, int value, size_t num) {
	unsigned char *data = (unsigned char *)ptr;
	for (size_t i = 0; i < num; ++i) {
		data[i] = (unsigned char)value;
	}
	return ptr;
}

void *memcpy(void *destination, const void *source, size_t num) {
	unsigned char *s = (unsigned char *)source;
	unsigned char *d = (unsigned char *)destination;
	for (size_t i = 0; i < num; ++i) {
		d[i] = s[i];
	}
	return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	unsigned char *p1 = (unsigned char *)ptr1;
	unsigned char *p2 = (unsigned char *)ptr2;
	for (size_t i = 0; i < num; ++i) {
		if (p1[i] != p2[i]) {
			return (int)p1[i] - (int)p2[i];
		}
	}
	return 0;
}
