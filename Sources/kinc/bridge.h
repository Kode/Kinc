#pragma once

// Temporary workaround to call some not-yet-translated C++ from C

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _Kinc_FramebufferOptions;

void Kinc_Bridge_G4_Internal_ChangeFramebuffer(int window_index, struct _Kinc_FramebufferOptions *frame);
void Kinc_Bridge_G4_Internal_SetAntialiasingSamples(int samples_per_pixel);
void Kinc_Bridge_G4_Internal_Init(int window_index, int depth_bits, int stencil_bits, bool vsync);

#ifdef __cplusplus
}
#endif
