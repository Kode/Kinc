#include "acceleration.h"

#include <memory.h>

void (*kinc_acceleration_callback)(float /*x*/, float /*y*/, float /*z*/) = NULL;
