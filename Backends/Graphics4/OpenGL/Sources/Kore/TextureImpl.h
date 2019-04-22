#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned int texture;
#ifdef KORE_ANDROID
	bool external_oes;
#endif
	uint8_t pixfmt;
} Kinc_G4_TextureImpl;

#ifdef __cplusplus
}
#endif
