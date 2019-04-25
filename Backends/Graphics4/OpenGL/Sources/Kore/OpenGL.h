#pragma once

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/Graphics4/Pipeline.h>
#include <Kinc/Graphics4/TextureUnit.h>

#ifdef __cplusplus
extern "C" {
#endif

int Kinc_G4_Internal_TextureAddressingU(kinc_g4_texture_unit_t unit);
int Kinc_G4_Internal_TextureAddressingV(kinc_g4_texture_unit_t unit);
int Kinc_G4_Internal_StencilFunc(Kinc_G4_CompareMode mode);

#ifdef __cplusplus
}
#endif
