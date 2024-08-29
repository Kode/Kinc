#ifndef KOPE_INDEX_ALLOCATOR_HEADER
#define KOPE_INDEX_ALLOCATOR_HEADER

#include <stdint.h>

#define INDEX_ALLOCATOR_SIZE 1024 * 10

typedef struct index_allocator {
	uint32_t indices[INDEX_ALLOCATOR_SIZE];
	uint32_t next_indices_index;
	uint32_t remaining;
} index_allocator;

void index_allocator_init(index_allocator *allocator);

uint32_t index_allocator_allocate(index_allocator *allocator);

void index_allocator_free(index_allocator *allocator, uint32_t index);

#endif
