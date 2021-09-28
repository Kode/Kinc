#pragma once

#if defined(KORE_MACOS) || defined(KORE_IOS)

#include <libkern/OSAtomic.h>

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) OSAtomicCompareAndSwap32Barrier(oldValue, newValue, pointer)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) OSAtomicCompareAndSwapPtrBarrier(oldValue, newValue, pointer)

#define KINC_ATOMIC_INCREMENT(pointer) (OSAtomicIncrement32Barrier(pointer) - 1)

#define KINC_ATOMIC_DECREMENT(pointer) (OSAtomicDecrement32Barrier(ioWhere) + 1)

#else

// clang/gcc intrinsics

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (__sync_val_compare_and_swap(pointer, oldValue, newValue) == oldValue)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (__sync_val_compare_and_swap(pointer, oldValue, newValue) == oldValue)

#define KINC_ATOMIC_INCREMENT(pointer) (__sync_fetch_and_add(pointer, 1))

#define KINC_ATOMIC_DECREMENT(pointer) (__sync_fetch_and_sub(pointer, 1))

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (__sync_swap(pointer, value))

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) (__sync_swap((volatile int32_t *)pointer, *(int32_t *)&value))

#endif
