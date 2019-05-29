#include "pch.h"

#include "Acceleration.h"

#include <memory.h>

void (*Kinc_AccelerationCallback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
