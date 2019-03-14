#pragma once

#include <intrin.h>

#define KORE_ATOMIC_COMPARE_EXCHANGE(pointer, oldValue, newValue) (_InterlockedCompareExchange((volatile long*) pointer, newValue, oldValue) == oldValue)

#define KORE_ATOMIC_COMPARE_EXCHANGE_POINTER(pointer, oldValue, newValue) (_InterlockedCompareExchangePointer(pointer, newValue, oldValue) == oldValue)

#define KORE_ATOMIC_INCREMENT(pointer) (_InterlockedIncrement((volatile long*)pointer) - 1)

#define KORE_ATOMIC_DECREMENT(pointer) (_InterlockedDecrement((volatile long*)pointer) + 1)
