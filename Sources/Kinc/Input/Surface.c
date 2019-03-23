#include "pch.h"

#include "Surface.h"

#include <memory.h>

void (*Kinc_Surface_TouchStartCallback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
void (*Kinc_Surface_MoveCallback)(int /*index*/, int /*x*/, int /*y*/) = NULL;
void (*Kinc_Surface_TouchEndCallback)(int /*index*/, int /*x*/, int /*y*/) = NULL;

void Kinc_Internal_Surface_TriggerTouchStart(int index, int x, int y) {
	if (Kinc_Surface_TouchStartCallback != NULL) {
		Kinc_Surface_TouchStartCallback(index, x, y);
	}
}

void Kinc_Internal_Surface_TriggerMove(int index, int x, int y) {
	if (Kinc_Surface_MoveCallback != NULL) {
		Kinc_Surface_MoveCallback(index, x, y);
	}
}

void Kinc_Internal_Surface_TriggerTouchEnd(int index, int x, int y) {
	if (Kinc_Surface_TouchEndCallback != NULL) {
		Kinc_Surface_TouchEndCallback(index, x, y);
	}
}
