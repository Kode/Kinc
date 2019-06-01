#pragma once

#include <Kore/ShaderImpl.h>
#include <Kore/TextureImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_texture_unit {
	Kinc_G4_TextureUnitImpl impl;
} kinc_g4_texture_unit_t;

#ifdef __cplusplus
}
#endif
