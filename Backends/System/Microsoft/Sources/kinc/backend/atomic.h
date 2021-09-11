#pragma once

#include <intrin.h>

#define KINC_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (_InterlockedCompareExchange((volatile long *)pointer, newValue, oldValue) == oldValue)

#define KINC_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (_InterlockedCompareExchangePointer(pointer, newValue, oldValue) == oldValue)

#define KINC_ATOMIC_INCREMENT(pointer) (_InterlockedIncrement((volatile long *)pointer) - 1)

#define KINC_ATOMIC_DECREMENT(pointer) (_InterlockedDecrement((volatile long *)pointer) + 1)

#define KINC_ATOMIC_EXCHANGE_32(pointer, value) (_InterlockedExchange(pointer, value))

#define KINC_ATOMIC_EXCHANGE_FLOAT(pointer, value) (_InterlockedExchange((volatile long *)pointer, *(long *)&value))

#define KINC_ATOMIC_EXCHANGE_DOUBLE(pointer, value) (_InterlockedExchange64((volatile __int64 *)pointer, *(__int64 *)&value))
