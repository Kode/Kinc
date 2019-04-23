#pragma once

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/Graphics4/PipelineState.h>
#include <Kinc/Graphics4/TextureUnit.h>

#ifdef __cplusplus
extern "C" {
#endif

int Kinc_G4_Internal_TextureAddressingU(Kinc_G4_TextureUnit unit);
int Kinc_G4_Internal_TextureAddressingV(Kinc_G4_TextureUnit unit);
int Kinc_G4_Internal_StencilFunc(Kinc_G4_CompareMode mode);

#ifdef __cplusplus
}
#endif
