#include "pch.h"

#include "acceleration.h"

#include <memory.h>

void (*Kinc_AccelerationCallback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
