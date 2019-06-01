#include "pch.h"

#include <kinc/threads/event.h>

void Kinc_Event_Create(Kinc_Event *event) {}

void Kinc_Event_Destroy(Kinc_Event *event) {}

void Kinc_Event_Signal(Kinc_Event *event) {}

void Kinc_Event_Wait(Kinc_Event *event) {}

bool Kinc_Event_TryToWait(Kinc_Event *event, double seconds) {
	return true;
}

void Kinc_Event_Reset(Kinc_Event *event) {}
