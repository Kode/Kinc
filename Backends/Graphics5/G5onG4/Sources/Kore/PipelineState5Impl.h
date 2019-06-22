#pragma once

#include <kinc/graphics4/pipeline.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	kinc_g4_pipeline_t pipe;
} PipelineState5Impl;

typedef struct {
	kinc_g4_constant_location_t location;
} ConstantLocation5Impl;

typedef struct {
	int nothing;
} AttributeLocation5Impl;

#ifdef __cplusplus
}
#endif
