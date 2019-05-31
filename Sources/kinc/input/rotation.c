#include "pch.h"

#include "rotation.h"

#include <memory.h>

void (*Kinc_RotationCallback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
