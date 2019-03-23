#include "pch.h"

#include "Rotation.h"

#include <memory.h>

void (*Kinc_RotationCallback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
