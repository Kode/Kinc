#include "pch.h"

#include "bridge.h"

#include "window.h"

#include <Kore/Graphics4/Graphics.h>

void kinc_bridge_g4_internal_change_framebuffer(int window_index, struct kinc_framebuffer_options *frame) {
	Kore::Graphics4::_changeFramebuffer(window_index, frame);
}

void kinc_bridge_g4_internal_set_antialiasing_samples(int samples_per_pixel) {
	Kore::Graphics4::setAntialiasingSamples(samples_per_pixel);
}
void kinc_bridge_g4_internal_init(int window_index, int depth_bits, int stencil_bits, bool vsync) {
	Kore::Graphics4::init(window_index, depth_bits, stencil_bits, vsync);
}
