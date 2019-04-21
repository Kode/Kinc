#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
#if defined(KORE_ANDROID) || defined(KORE_PI)
	uint16_t *shortData;
#endif
	int *data;
	int myCount;
	unsigned bufferId;
} Kinc_G4_IndexBufferImpl;

#ifdef __cplusplus
}
#endif
