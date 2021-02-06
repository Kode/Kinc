#pragma once

#include <objc/runtime.h>

typedef struct {
	id _buffer;
	int lastStart;
	int lastCount;
	int mySize;
} ConstantBuffer5Impl;
