#pragma once

#include <objc/runtime.h>

typedef struct {
	id _buffer;
	int lastStart;
	int lastCount;
	int mySize;
	//const bool transposeMat3 = true;
	//const bool transposeMat4 = true;
} ConstantBuffer5Impl;
