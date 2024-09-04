// (C) Sebastian Aaltonen 2023
// MIT License (see file: LICENSE)
// C99 conversion by Aarni Gratseff
// https://github.com/aarni57/offalloc

#ifndef OFFALLOC_H
#define OFFALLOC_H

#include "stdint.h"

#define OA_NUM_TOP_BINS 32
#define OA_BINS_PER_LEAF 8
#define OA_NUM_LEAF_BINS OA_NUM_TOP_BINS * OA_BINS_PER_LEAF

#define OA_NO_SPACE 0xffffffff

typedef uint16_t oa_index_t;
#define OA_INVALID_INDEX (oa_index_t)0xffff
#define OA_UNUSED (oa_index_t)0xffff

typedef struct oa_allocation_t {
    uint32_t offset;
    oa_index_t index; // internal: node index
} oa_allocation_t;

typedef struct ao_storage_report_t {
    uint32_t total_free_space;
    uint32_t largest_free_region;
} oa_storage_report_t;

typedef struct ao_region_t {
    uint32_t size;
    uint32_t count;
} oa_region_t;

typedef struct ao_storage_report_full_t {    
    oa_region_t free_regions[OA_NUM_LEAF_BINS];
} oa_storage_report_full_t;

typedef struct ao_node_t {
    uint32_t data_offset;
    uint32_t data_size; // 'used' flag stored as high bit
    oa_index_t bin_list_prev;
    oa_index_t bin_list_next;
    oa_index_t neighbor_prev;
    oa_index_t neighbor_next;
} oa_node_t;

typedef struct ao_allocator_t {
    uint32_t size;
    uint32_t max_allocs;
    uint32_t free_storage;

    uint32_t used_bins_top;
    uint8_t used_bins[OA_NUM_TOP_BINS];
    oa_index_t bin_indices[OA_NUM_LEAF_BINS];

    oa_node_t *nodes;
    oa_index_t *free_nodes;
    uint32_t free_offset;
} oa_allocator_t;

//

int oa_create(oa_allocator_t *self, uint32_t size, uint32_t max_allocs);
void oa_destroy(oa_allocator_t *self);

int oa_allocate(oa_allocator_t *self, uint32_t size, oa_allocation_t *allocation);
void oa_free(oa_allocator_t *self, oa_allocation_t *allocation);

uint32_t oa_allocation_size(oa_allocator_t *self, const oa_allocation_t *allocation);
void oa_storage_report(const oa_allocator_t *self, oa_storage_report_t *report);
void oa_storage_report_full(const oa_allocator_t *self, oa_storage_report_full_t *report);

#endif
