#pragma once

// Temporary workaround to call some not-yet-translated C++ from C

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _Kore_FramebufferOptions;

void Kore_Bridge_G4_Internal_ChangeFramebuffer(int window_index, struct _Kore_FramebufferOptions *frame);
void Kore_Bridge_G4_Internal_SetAntialiasingSamples(int samples_per_pixel);
void Kore_Bridge_G4_Internal_Init(int window_index, int depth_bits, int stencil_bits, bool vsync);

#ifdef __cplusplus
}
#endif
