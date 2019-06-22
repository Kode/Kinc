#include "pch.h"

#include "rotation.h"

#include <memory.h>

void (*kinc_rotation_callback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
