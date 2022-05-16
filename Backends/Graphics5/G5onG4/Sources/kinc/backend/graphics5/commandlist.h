#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g5_pipeline;

typedef struct {
	struct kinc_g5_pipeline *_currentPipeline;
	int _indexCount;
	char commands[1024 * 8];
	int commandIndex;
	bool closed;
} CommandList5Impl;

#ifdef __cplusplus
}
#endif
