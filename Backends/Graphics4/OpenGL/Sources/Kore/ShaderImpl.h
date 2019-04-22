#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned _glid;
	const char *source;
	size_t length;
	bool fromSource;
} Kinc_G4_ShaderImpl;

typedef struct {
	int location;
	unsigned type;
} Kinc_G4_ConstantLocationImpl;

typedef struct {
	int unit;
} Kinc_G4_TextureUnitImpl;

#ifdef __cplusplus
}
#endif
