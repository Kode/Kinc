#include "pch.h"

#include "Graphics.h"

static int antialiasing_samples;

int Kinc_G4_AntialiasingSamples() {
	return antialiasing_samples;
}

void Kinc_G4_SetAntialiasingSamples(int samples) {
	antialiasing_samples = samples;
}
