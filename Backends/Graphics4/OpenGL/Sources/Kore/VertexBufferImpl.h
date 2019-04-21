#pragma once

#include <Kinc/Graphics4/VertexStructure.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	float *data;
	int myCount;
	int myStride;
	unsigned bufferId;
	unsigned usage;
	int sectionStart;
	int sectionSize;
	//#if defined KORE_ANDROID || defined KORE_HTML5 || defined KORE_TIZEN
	Kinc_G4_VertexStructure structure;
	//#endif
	int instanceDataStepRate;
#ifndef NDEBUG
	bool initialized;
#endif
} Kinc_G4_VertexBufferImpl;

#ifdef __cplusplus
}
#endif
