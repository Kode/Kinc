// (C) Sebastian Aaltonen 2023
// MIT License (see file: LICENSE)
// C99 conversion by Aarni Gratseff
// https://github.com/aarni57/offalloc

#include "offalloc.h"

#include <stdlib.h>

#if !defined(NDEBUG)
#   define OA_DEBUG

#   include <assert.h>
#   define oa_assert(x) assert(x)
#else
#   define oa_assert(x)
#endif

#if defined(OA_VERBOSE)
#   include <stdio.h>
#endif

static uint32_t lzcnt_nonzero(uint32_t v)
{
#if defined(_MSC_VER)
    unsigned long retVal;
    _BitScanReverse(&retVal, v);
    return 31 - retVal;
#else
    return __builtin_clz(v);
#endif
}

static uint32_t tzcnt_nonzero(uint32_t v)
{
#if defined(_MSC_VER)
    unsigned long retVal;
    _BitScanForward(&retVal, v);
    return retVal;
#else
    return __builtin_ctz(v);
#endif
}

#define SMALLFLOAT_MANTISSA_BITS 3
#define SMALLFLOAT_MANTISSA_VALUE (1 << SMALLFLOAT_MANTISSA_BITS)
#define SMALLFLOAT_MANTISSA_MASK (SMALLFLOAT_MANTISSA_VALUE - 1)

// Bin sizes follow floating point (exponent + mantissa) distribution
// (piecewise linear log approx)
// This ensures that for each size class,
// the average overhead percentage stays the same
static uint32_t smallfloat_uint_to_float_round_up(uint32_t size)
{
    uint32_t exp = 0;
    uint32_t mantissa = 0;

    if (size < SMALLFLOAT_MANTISSA_VALUE) {
        // Denorm: 0..(MANTISSA_VALUE-1)
        mantissa = size;
    } else {
        // Normalized: Hidden high bit always 1. Not stored. Just like float.
        uint32_t leadingZeros = lzcnt_nonzero(size);
        uint32_t highestSetBit = 31 - leadingZeros;

        uint32_t mantissaStartBit = highestSetBit - SMALLFLOAT_MANTISSA_BITS;
        exp = mantissaStartBit + 1;
        mantissa = (size >> mantissaStartBit) & SMALLFLOAT_MANTISSA_MASK;

        uint32_t lowBitsMask = (1 << mantissaStartBit) - 1;

        // Round up!
        if ((size & lowBitsMask) != 0)
            mantissa++;
    }

    // + allows mantissa->exp overflow for round up
    return (exp << SMALLFLOAT_MANTISSA_BITS) + mantissa;
}

static uint32_t smallfloat_uint_to_float_round_down(uint32_t size)
{
    uint32_t exp = 0;
    uint32_t mantissa = 0;

    if (size < SMALLFLOAT_MANTISSA_VALUE) {
        // Denorm: 0..(MANTISSA_VALUE-1)
        mantissa = size;
    } else {
        // Normalized: Hidden high bit always 1. Not stored. Just like float.
        uint32_t leadingZeros = lzcnt_nonzero(size);
        uint32_t highestSetBit = 31 - leadingZeros;

        uint32_t mantissaStartBit = highestSetBit - SMALLFLOAT_MANTISSA_BITS;
        exp = mantissaStartBit + 1;
        mantissa = (size >> mantissaStartBit) & SMALLFLOAT_MANTISSA_MASK;
    }

    return (exp << SMALLFLOAT_MANTISSA_BITS) | mantissa;
}

static uint32_t smallfloat_float_to_uint(uint32_t floatValue)
{
    uint32_t exponent = floatValue >> SMALLFLOAT_MANTISSA_BITS;
    uint32_t mantissa = floatValue & SMALLFLOAT_MANTISSA_MASK;
    if (exponent == 0) // Denorms
        return mantissa;
    else
        return (mantissa | SMALLFLOAT_MANTISSA_VALUE) << (exponent - 1);
}

// Utility functions
static uint32_t find_lowest_set_bit_after(uint32_t bitmask,
    uint32_t start_bit_index)
{
    uint32_t mask_before_start_index = (1 << start_bit_index) - 1;
    uint32_t mask_after_start_index = ~mask_before_start_index;
    uint32_t bits_after = bitmask & mask_after_start_index;
    if (bits_after == 0) return OA_NO_SPACE;
    return tzcnt_nonzero(bits_after);
}

//

static const struct ao_node_t AO_NODE_DEFAULTS = {
    .data_offset = 0,
    .data_size = 0,
    .bin_list_prev = OA_INVALID_INDEX,
    .bin_list_next = OA_INVALID_INDEX,
    .neighbor_prev = OA_INVALID_INDEX,
    .neighbor_next = OA_INVALID_INDEX,
};

#define OA_TOP_BINS_INDEX_SHIFT 3
#define OA_LEAF_BINS_INDEX_MASK 0x7

#define OA_NODE_USED_FLAG (1 << 31)
#define OA_NODE_DATA_SIZE_MASK ~OA_NODE_USED_FLAG

//

static uint32_t insert_node_into_bin(oa_allocator_t *self, uint32_t size,
    uint32_t data_offset)
{
    // Round down to bin index to ensure that bin >= alloc
    uint32_t bin_index = smallfloat_uint_to_float_round_down(size);
    oa_assert(bin_index < OA_NUM_LEAF_BINS);

    uint32_t top_bin_index = bin_index >> OA_TOP_BINS_INDEX_SHIFT;
    uint32_t leaf_bin_index = bin_index & OA_LEAF_BINS_INDEX_MASK;

    // Bin was empty before?
    if (self->bin_indices[bin_index] == OA_UNUSED) {
        // Set bin mask bits
        self->used_bins[top_bin_index] |= 1 << leaf_bin_index;
        self->used_bins_top |= 1 << top_bin_index;
    }

    // Take a freelist node and insert on top of the bin linked list
    // (next = old top)
    uint32_t top_node_index = self->bin_indices[bin_index];
    oa_assert(self->free_offset < self->max_allocs);
    uint32_t node_index = self->free_nodes[self->free_offset--];
    oa_assert(node_index < self->max_allocs);

#if defined(OA_VERBOSE)
    printf("Getting node %u from freelist[%u]\n", node_index,
        self->free_offset + 1);
#endif

    self->nodes[node_index] = AO_NODE_DEFAULTS;
    self->nodes[node_index].data_offset = data_offset;
    self->nodes[node_index].data_size = size;
    self->nodes[node_index].bin_list_next = top_node_index;

    if (top_node_index != OA_UNUSED) {
        oa_assert(top_node_index < self->max_allocs);
        self->nodes[top_node_index].bin_list_prev = node_index;
    }

    self->bin_indices[bin_index] = node_index;

    self->free_storage += size;

#if defined(OA_VERBOSE)
    printf("Free storage: %u (+%u) (insert_node_into_bin)\n",
        self->free_storage, size);
#endif

    return node_index;
}

static void remove_node_from_bin(oa_allocator_t *self, uint32_t node_index)
{
    oa_assert(node_index < self->max_allocs);
    oa_node_t *node = &self->nodes[node_index];

    if (node->bin_list_prev != OA_UNUSED) {
        // Easy case: We have previous node.
        // Just remove this node from the middle of the list.
        self->nodes[node->bin_list_prev].bin_list_next = node->bin_list_next;
        if (node->bin_list_next != OA_UNUSED)
            self->nodes[node->bin_list_next].bin_list_prev =
                node->bin_list_prev;
    } else {
        // Hard case: We are the first node in a bin. Find the bin.

        // Round down to bin index to ensure that bin >= alloc
        uint32_t bin_index = smallfloat_uint_to_float_round_down(
            node->data_size & OA_NODE_DATA_SIZE_MASK);

        uint32_t top_bin_index = bin_index >> OA_TOP_BINS_INDEX_SHIFT;
        uint32_t leaf_bin_index = bin_index & OA_LEAF_BINS_INDEX_MASK;

        self->bin_indices[bin_index] = node->bin_list_next;
        if (node->bin_list_next != OA_UNUSED)
            self->nodes[node->bin_list_next].bin_list_prev = OA_UNUSED;

        // Bin empty?
        if (self->bin_indices[bin_index] == OA_UNUSED) {
            // Remove a leaf bin mask bit
            self->used_bins[top_bin_index] &= ~(1 << leaf_bin_index);

            // All leaf bins empty?
            if (self->used_bins[top_bin_index] == 0)
                // Remove a top bin mask bit
                self->used_bins_top &= ~(1 << top_bin_index);
        }
    }

    // Insert the node to freelist
#if defined(OA_VERBOSE)
    printf("Putting node %u into freelist[%u] (remove_node_from_bin)\n",
        node_index, self->free_offset + 1);
#endif
    oa_assert(self->free_offset + 1 < self->max_allocs);
    self->free_nodes[++self->free_offset] = node_index;

    self->free_storage -= node->data_size & OA_NODE_DATA_SIZE_MASK;
#if defined(OA_VERBOSE)
    printf("Free storage: %u (-%u) (remove_node_from_bin)\n",
        self->free_storage, node->data_size & OA_NODE_DATA_SIZE_MASK);
#endif
}

//

int oa_create(oa_allocator_t *self, uint32_t size, uint32_t max_allocs)
{
    oa_assert(!self->nodes);
    oa_assert(!self->free_nodes);
    oa_assert(self->used_bins_top == 0);

    self->size = size;
    self->max_allocs = max_allocs;

    self->free_offset = self->max_allocs - 1;

    for (uint32_t i = 0 ; i < OA_NUM_LEAF_BINS; ++i)
        self->bin_indices[i] = OA_INVALID_INDEX;

    self->nodes = (oa_node_t *)malloc(sizeof(oa_node_t) * self->max_allocs);
    if (!self->nodes) {
        return -1;
    }

    self->free_nodes = (oa_index_t *)malloc(
        sizeof(oa_index_t) * self->max_allocs);
    if (!self->free_nodes) {
        free(self->nodes);
        self->nodes = NULL;
        return -1;
    }

    for (uint32_t i = 0; i < self->max_allocs; ++i) {
        self->nodes[i] = AO_NODE_DEFAULTS;
    }

    // Freelist is a stack. Nodes in inverse order so that [0] pops first.
    for (uint32_t i = 0; i < self->max_allocs; ++i)
        self->free_nodes[i] = self->max_allocs - i - 1;

    // Start state: Whole storage as one big node
    // Algorithm will split remainders and push them back as smaller nodes
    insert_node_into_bin(self, self->size, 0);

    return 0;
}

void oa_destroy(oa_allocator_t *self)
{
#if defined(OA_DEBUG)
    remove_node_from_bin(self, 0);
    oa_assert(self->free_offset == self->max_allocs - 1);
    oa_assert(self->free_storage == 0);
#endif

    free(self->free_nodes);
    self->free_nodes = NULL;

    free(self->nodes);
    self->nodes = NULL;
}

int oa_allocate(oa_allocator_t *self, uint32_t size,
    oa_allocation_t *allocation)
{
    oa_assert(size != 0);

    // Out of allocations?
    if (self->free_offset == 0) {
        allocation->offset = OA_NO_SPACE;
        allocation->index = OA_INVALID_INDEX;
        return -1;
    }

    // Round up to bin index to ensure that alloc >= bin
    // Gives us min bin index that fits the size
    uint32_t min_bin_index = smallfloat_uint_to_float_round_up(size);

    uint32_t min_top_bin_index = min_bin_index >> OA_TOP_BINS_INDEX_SHIFT;
    uint32_t min_leaf_bin_index = min_bin_index & OA_LEAF_BINS_INDEX_MASK;

    uint32_t top_bin_index = min_top_bin_index;
    uint32_t leaf_bin_index = OA_NO_SPACE;

    // If top bin exists, scan its leaf bin. This can fail (NO_SPACE).
    if (self->used_bins_top & (1 << top_bin_index))
        leaf_bin_index = find_lowest_set_bit_after(
            self->used_bins[top_bin_index], min_leaf_bin_index);

    // If we didn't find space in top bin, we search top bin from +1
    if (leaf_bin_index == OA_NO_SPACE) {
        top_bin_index = find_lowest_set_bit_after(
            self->used_bins_top, min_top_bin_index + 1);

        // Out of space?
        if (top_bin_index == OA_NO_SPACE) {
#if defined(OA_VERBOSE)
            printf("oa_allocate: No space; trying to allocate %u\n", size);
#endif
            allocation->offset = OA_NO_SPACE;
            allocation->index = OA_INVALID_INDEX;
            return -1;
        }

        // All leaf bins here fit the alloc, since the top bin was rounded up.
        // Start leaf search from bit 0. NOTE: This search can't fail since at
        // least one leaf bit was set because the top bit was set.
        leaf_bin_index = tzcnt_nonzero(self->used_bins[top_bin_index]);
    }

    uint32_t bin_index =
        (top_bin_index << OA_TOP_BINS_INDEX_SHIFT) | leaf_bin_index;
    oa_assert(bin_index < OA_NUM_LEAF_BINS);

    // Pop the top node of the bin. Bin top = node->next.
    uint32_t node_index = self->bin_indices[bin_index];
    oa_assert(node_index < self->max_allocs);
    oa_node_t *node = &self->nodes[node_index];
    uint32_t node_total_size = node->data_size & OA_NODE_DATA_SIZE_MASK;
    node->data_size = size | OA_NODE_USED_FLAG;
    self->bin_indices[bin_index] = node->bin_list_next;

    if (node->bin_list_next != OA_INVALID_INDEX)
        self->nodes[node->bin_list_next].bin_list_prev = OA_INVALID_INDEX;

    oa_assert(self->free_storage >= node_total_size);
    self->free_storage -= node_total_size;

#if defined(OA_VERBOSE)
    printf("Free storage: %u (-%u) (oa_allocate)\n", self->free_storage,
        node_total_size);
#endif

    // Bin empty?
    if (self->bin_indices[bin_index] == OA_UNUSED) {
        // Remove a leaf bin mask bit
        self->used_bins[top_bin_index] &= ~(1 << leaf_bin_index);

        // All leaf bins empty?
        if (self->used_bins[top_bin_index] == 0)
            // Remove a top bin mask bit
            self->used_bins_top &= ~(1 << top_bin_index);
    }

    // Push back remainder N elements to a lower bin
    oa_assert(node_total_size >= size);
    uint32_t remainder_size = node_total_size - size;
    if (remainder_size != 0) {
        uint32_t new_node_index = insert_node_into_bin(self, remainder_size,
            node->data_offset + size);

        // Link nodes next to each other so that we can merge them
        // later if both are free. And update the old next neighbor to point
        // to the new node (in middle)
        if (node->neighbor_next != OA_UNUSED)
            self->nodes[node->neighbor_next].neighbor_prev = new_node_index;

        self->nodes[new_node_index].neighbor_prev = node_index;
        self->nodes[new_node_index].neighbor_next = node->neighbor_next;
        node->neighbor_next = new_node_index;
    }

    allocation->offset = node->data_offset;
    allocation->index = node_index;
    return 0;
}

void oa_free(oa_allocator_t* self, oa_allocation_t *allocation)
{
    if (allocation->index == OA_INVALID_INDEX) return;
    if (!self->nodes) return;

    uint32_t node_index = allocation->index;
    oa_node_t *node = &self->nodes[node_index];

    // Double delete check
    oa_assert(node->data_size & OA_NODE_USED_FLAG);

    // Merge with neighbors...
    uint32_t offset = node->data_offset;
    uint32_t size = node->data_size & OA_NODE_DATA_SIZE_MASK;

    oa_assert(node->neighbor_prev != node_index);
    if ((node->neighbor_prev != OA_UNUSED) &&
        (!(self->nodes[node->neighbor_prev].data_size & OA_NODE_USED_FLAG))) {
        // Previous (contiguous) free node:
        // Change offset to previous node offset. Sum sizes
        oa_node_t *prev_node = &self->nodes[node->neighbor_prev];
        offset = prev_node->data_offset;
        size += prev_node->data_size & OA_NODE_DATA_SIZE_MASK;

        // Remove node from the bin linked list and put it in the freelist
        remove_node_from_bin(self, node->neighbor_prev);

        oa_assert(prev_node->neighbor_next == node_index);
        node->neighbor_prev = prev_node->neighbor_prev;
    }

    oa_assert(node->neighbor_next != node_index);
    if ((node->neighbor_next != OA_UNUSED) &&
        (!(self->nodes[node->neighbor_next].data_size & OA_NODE_USED_FLAG))) {
        // Next (contiguous) free node: Offset remains the same. Sum sizes.
        oa_node_t *next_node = &self->nodes[node->neighbor_next];
        size += next_node->data_size & OA_NODE_DATA_SIZE_MASK;

        // Remove node from the bin linked list and put it in the freelist
        remove_node_from_bin(self, node->neighbor_next);

        oa_assert(next_node->neighbor_prev == node_index);
        node->neighbor_next = next_node->neighbor_next;
    }

    uint32_t neighbor_next = node->neighbor_next;
    uint32_t neighbor_prev = node->neighbor_prev;

    // Insert the removed node to freelist
#if defined(OA_VERBOSE)
    printf("Putting node %u into freelist[%u] (oa_free)\n", node_index,
        self->free_offset + 1);
#endif

    oa_assert(self->free_offset + 1 < self->max_allocs);
    self->free_nodes[++self->free_offset] = node_index;

    // Insert the (combined) free node to bin
    uint32_t combined_node_index = insert_node_into_bin(self, size, offset);

    // Connect neighbors with the new combined node
    if (neighbor_next != OA_UNUSED) {
        self->nodes[combined_node_index].neighbor_next = neighbor_next;
        self->nodes[neighbor_next].neighbor_prev = combined_node_index;
    }

    if (neighbor_prev != OA_UNUSED) {
        self->nodes[combined_node_index].neighbor_prev = neighbor_prev;
        self->nodes[neighbor_prev].neighbor_next = combined_node_index;
    }
}

uint32_t oa_allocation_size(oa_allocator_t *self,
    const oa_allocation_t *allocation)
{
    if (allocation->index == OA_INVALID_INDEX) return 0;
    if (!self->nodes) return 0;
    return self->nodes[allocation->index].data_size & OA_NODE_DATA_SIZE_MASK;
}

void oa_storage_report(const oa_allocator_t *self, oa_storage_report_t *report)
{
    uint32_t largest_free_region = 0;
    uint32_t free_storage = 0;

    // Out of allocations? -> Zero free space
    if (self->free_offset != 0) {
        free_storage = self->free_storage;
        if (self->used_bins_top) {
            uint32_t top_bin_index =
                31 - lzcnt_nonzero(self->used_bins_top);
            uint32_t leaf_bin_index =
                31 - lzcnt_nonzero(self->used_bins[top_bin_index]);
            largest_free_region = smallfloat_float_to_uint(
                (top_bin_index << OA_TOP_BINS_INDEX_SHIFT) | leaf_bin_index);
            oa_assert(free_storage >= largest_free_region);
        }
    }

    report->total_free_space = free_storage;
    report->largest_free_region = largest_free_region;
}

void oa_storage_report_full(const oa_allocator_t *self,
    oa_storage_report_full_t *report)
{
    for (uint32_t i = 0; i < OA_NUM_LEAF_BINS; ++i) {
        uint32_t count = 0;
        uint32_t node_index = self->bin_indices[i];
        while (node_index != OA_UNUSED) {
            oa_assert(node_index < self->max_allocs);
            node_index = self->nodes[node_index].bin_list_next;
            count++;
        }

        report->free_regions[i].size = smallfloat_float_to_uint(i);
        report->free_regions[i].count = count;
    }
}
