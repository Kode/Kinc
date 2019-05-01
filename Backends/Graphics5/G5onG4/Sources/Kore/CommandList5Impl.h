#pragma once

#include "pch.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_pipeline;

typedef struct {
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;
	int64_t commands[1024];
	int commandIndex;
	bool closed;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
