#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long size_t;
#define NULL 0

void *malloc(size_t size);

void *realloc(void *mem, size_t size);

void free(void *mem);

void *memset(void *ptr, int value, size_t num);

void *memcpy(void *destination, const void *source, size_t num);

int memcmp(const void *ptr1, const void *ptr2, size_t num);

#ifdef __cplusplus
}
#endif
