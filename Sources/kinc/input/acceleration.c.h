#include "acceleration.h"

#include <memory.h>

void (*acceleration_callback)(float /*x*/, float /*y*/, float /*z*/) = NULL;

void kinc_acceleration_set_callback(void (*value)(float /*x*/, float /*y*/, float /*z*/)) {
	acceleration_callback = value;
}

void kinc_internal_on_acceleration(float x, float y, float z) {
	if (acceleration_callback != NULL) {
		acceleration_callback(x, y, z);
	}
}
