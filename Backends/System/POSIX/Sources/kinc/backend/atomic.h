#pragma once

#include <kinc/global.h>

#if defined(KORE_MACOS) || defined(KORE_IOS)

#include <libkern/OSAtomic.h>

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) OSAtomicCompareAndSwap32Barrier(oldValue, newValue, pointer)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) OSAtomicCompareAndSwapPtrBarrier(oldValue, newValue, pointer)

#define KINC_ATOMIC_INCREMENT(pointer) (OSAtomicIncrement32Barrier(pointer) - 1)

#define KINC_ATOMIC_DECREMENT(pointer) (OSAtomicDecrement32Barrier(pointer) + 1)

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (__sync_swap(pointer, value))

KINC_INLINE void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_swap((volatile int32_t *)pointer, *(int32_t *)&value);
}

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) kinc_atomic_exchange_float(pointer, value)

#else

// clang/gcc intrinsics

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (__sync_val_compare_and_swap(pointer, oldValue, newValue) == oldValue)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (__sync_val_compare_and_swap(pointer, oldValue, newValue) == oldValue)

#define KINC_ATOMIC_INCREMENT(pointer) (__sync_fetch_and_add(pointer, 1))

#define KINC_ATOMIC_DECREMENT(pointer) (__sync_fetch_and_sub(pointer, 1))

#ifdef __clang__

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (__sync_swap(pointer, value))

KINC_INLINE void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_swap((volatile int32_t *)pointer, *(int32_t *)&value);
}

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) kinc_atomic_exchange_float(pointer, value)

#else

// Beware, __sync_lock_test_and_set is not a full barrier and can have platform-specific weirdness

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (__sync_lock_test_and_set(pointer, value))

KINC_INLINE void kinc_atomic_exchange_float(volatile float *pointer, float value) {
	__sync_lock_test_and_set((volatile int32_t *)pointer, *(int32_t *)&value);
}

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) kinc_atomic_exchange_float(pointer, value)

#endif

#endif
