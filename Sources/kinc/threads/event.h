#pragma once

#include <Kore/EventImpl.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Kinc_EventImpl impl;
} Kinc_Event;

void Kinc_Event_Create(Kinc_Event *event);
void Kinc_Event_Destroy(Kinc_Event *event);
void Kinc_Event_Signal(Kinc_Event *event);
void Kinc_Event_Wait(Kinc_Event *event);
bool Kinc_Event_TryToWait(Kinc_Event *event, double seconds);
void Kinc_Event_Reset(Kinc_Event *event);

#ifdef __cplusplus
}
#endif
