#ifndef KOPE_D3D12_PIPELINE_FUNCTIONS_HEADER
#define KOPE_D3D12_PIPELINE_FUNCTIONS_HEADER

#include "pipeline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void kope_d3d12_pipeline_init(kope_d3d12_pipeline *pipe);

void kope_d3d12_pipeline_destroy(kope_d3d12_pipeline *pipe);

#ifdef __cplusplus
}
#endif

#endif
