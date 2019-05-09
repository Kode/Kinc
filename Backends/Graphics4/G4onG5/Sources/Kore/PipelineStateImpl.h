#pragma once

#include <Kinc/Graphics5/Pipeline.h>
#include <Kinc/Graphics5/ConstantLocation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	// PipelineStateImpl();
	kinc_g5_pipeline_t _pipeline;
} Kinc_G4_PipelineStateImpl;

typedef struct {
	kinc_g5_constant_location_t _location;
} Kinc_G4_ConstantLocationImpl;

typedef struct {
} Kinc_G4_AttributeLocationImpl;

#ifdef __cplusplus
}
#endif