#include "pch.h"

#include "Bridge.h"

#include "Window.h"

#include <Kore/Graphics4/Graphics.h>

void Kore_Bridge_G4_Internal_ChangeFramebuffer(int window_index, struct _Kore_FramebufferOptions *frame) {
	Kore::Graphics4::_changeFramebuffer(window_index, frame);
}

void Kore_Bridge_G4_Internal_SetAntialiasingSamples(int samples_per_pixel) {
	Kore::Graphics4::setAntialiasingSamples(samples_per_pixel);
}
void Kore_Bridge_G4_Internal_Init(int window_index, int depth_bits, int stencil_bits, bool vsync) {
	Kore::Graphics4::init(window_index, depth_bits, stencil_bits, vsync);
}
