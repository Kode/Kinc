#include "indexallocator.h"

void index_allocator_init(index_allocator *allocator) {
	for (uint32_t i = 0; i < INDEX_ALLOCATOR_SIZE; ++i) {
		allocator->indices[i] = i;
	}
	allocator->next_indices_index = 0;
	allocator->remaining = INDEX_ALLOCATOR_SIZE;
}

uint32_t index_allocator_allocate(index_allocator *allocator) {
	if (allocator->remaining == 0) {
		return 0xffffffff;
	}

	uint32_t index = allocator->indices[allocator->next_indices_index];

	allocator->next_indices_index += 1;
	if (allocator->next_indices_index >= INDEX_ALLOCATOR_SIZE) {
		allocator->next_indices_index = 0;
	}

	allocator->remaining -= 1;

	return index;
}

void index_allocator_free(index_allocator *allocator, uint32_t index) {
	uint32_t indices_index = allocator->next_indices_index + allocator->remaining;
	if (indices_index >= INDEX_ALLOCATOR_SIZE) {
		indices_index -= INDEX_ALLOCATOR_SIZE;
	}

	allocator->indices[indices_index] = index;

	allocator->remaining += 1;
}
