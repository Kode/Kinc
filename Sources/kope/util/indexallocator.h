#ifndef KOPE_INDEX_ALLOCATOR_HEADER
#define KOPE_INDEX_ALLOCATOR_HEADER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KOPE_INDEX_ALLOCATOR_SIZE 1024 * 10

typedef struct kope_index_allocator {
	uint32_t indices[KOPE_INDEX_ALLOCATOR_SIZE];
	uint32_t next_indices_index;
	uint32_t remaining;
} kope_index_allocator;

void kope_index_allocator_init(kope_index_allocator *allocator);

uint32_t kope_index_allocator_allocate(kope_index_allocator *allocator);

void kope_index_allocator_free(kope_index_allocator *allocator, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
