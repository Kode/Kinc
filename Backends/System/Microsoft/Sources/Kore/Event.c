#include "pch.h"

#include <Kinc/Threads/Event.h>

#include <Windows.h>

void Kinc_Event_Create(Kinc_Event *event) {
	event->impl.event = CreateEvent(0, 0, 0, 0);
}

void Kinc_Event_Destroy(Kinc_Event *event) {
	CloseHandle(event->impl.event);
}

void Kinc_Event_Signal(Kinc_Event *event) {
	SetEvent(event->impl.event);
}

void Kinc_Event_Wait(Kinc_Event *event) {
	WaitForSingleObject(event->impl.event, INFINITE);
}

bool Kinc_Event_TryToWait(Kinc_Event *event, double seconds) {
	return WaitForSingleObject(event->impl.event, (DWORD)(seconds * 1000.0)) != WAIT_TIMEOUT;
}

void Kinc_Event_Reset(Kinc_Event *event) {
	ResetEvent(event->impl.event);
}
