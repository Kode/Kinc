#pragma once

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/textureunit.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KINC_KONG
int Kinc_G4_Internal_TextureAddressingU(uint32_t unit);
int Kinc_G4_Internal_TextureAddressingV(uint32_t unit);
#else
int Kinc_G4_Internal_TextureAddressingU(kinc_g4_texture_unit_t unit);
int Kinc_G4_Internal_TextureAddressingV(kinc_g4_texture_unit_t unit);
#endif
int Kinc_G4_Internal_StencilFunc(kinc_g4_compare_mode_t mode);

#ifdef __cplusplus
}
#endif
