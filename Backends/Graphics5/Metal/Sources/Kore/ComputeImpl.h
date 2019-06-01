#pragma once

#include <objc/runtime.h>

#include <stdint.h>

typedef struct {
	uint32_t _offset;
} ComputeConstantLocationImpl;

typedef struct {
	uint32_t _index;
} ComputeTextureUnitImpl;

typedef struct {
	char name[1024];
	id _function;
	id _pipeline;
	id _reflection;
} ComputeShaderImpl;
