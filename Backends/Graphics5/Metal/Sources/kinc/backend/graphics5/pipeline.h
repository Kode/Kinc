#pragma once

#include <objc/runtime.h>

struct kinc_g5_shader;

typedef struct {
	struct kinc_g5_shader *vertexShader;
	struct kinc_g5_shader *fragmentShader;
	id _pipeline;
	id _pipelineDepth;
	id _reflection;
	id _depthStencil;
	id _depthStencilNone;
	//void _set();
} PipelineState5Impl;

typedef struct {
	int a;
} ComputePipelineState5Impl;

typedef struct {
	int vertexOffset;
	int fragmentOffset;
} ConstantLocation5Impl;
