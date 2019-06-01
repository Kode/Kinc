#pragma once

// Temporary workaround to call some not-yet-translated C++ from C

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_framebuffer_options;

void kinc_bridge_g4_internal_change_framebuffer(int window_index, struct kinc_framebuffer_options *frame);
void kinc_bridge_g4_internal_set_antialiasing_samples(int samples_per_pixel);
void kinc_bridge_g4_internal_init(int window_index, int depth_bits, int stencil_bits, bool vsync);

#ifdef __cplusplus
}
#endif
