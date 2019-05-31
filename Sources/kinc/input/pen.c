#include "pch.h"

#include "pen.h"

#include <memory.h>

void (*Kinc_Pen_PressCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
void (*Kinc_Pen_MoveCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;
void (*Kinc_Pen_ReleaseCallback)(int /*window*/, int /*x*/, int /*y*/, float /*pressure*/) = NULL;

void Kinc_Internal_Pen_TriggerPress(int window, int x, int y, float pressure) {
	if (Kinc_Pen_PressCallback != NULL) {
		Kinc_Pen_PressCallback(window, x, y, pressure);
	}
}

void Kinc_Internal_Pen_TriggerMove(int window, int x, int y, float pressure) {
	if (Kinc_Pen_MoveCallback != NULL) {
		Kinc_Pen_MoveCallback(window, x, y, pressure);
	}
}

void Kinc_Internal_Pen_TriggerRelease(int window, int x, int y, float pressure) {
	if (Kinc_Pen_ReleaseCallback != NULL) {
		Kinc_Pen_ReleaseCallback(window, x, y, pressure);
	}
}
