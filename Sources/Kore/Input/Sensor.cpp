#include "Sensor.h"

#include <kinc/input/acceleration.h>
#include <kinc/input/rotation.h>

using namespace Kore;

namespace {
	Sensor accelerometer;
	Sensor gyroscope;
	bool initialized = false;

	void accelerate(float x, float y, float z) {
		if (accelerometer.Changed != nullptr) {
			accelerometer.Changed(x, y, z);
		}
	}

	void rotate(float x, float y, float z) {
		if (gyroscope.Changed != nullptr) {
			gyroscope.Changed(x, y, z);
		}
	}
}

Sensor *Sensor::the(SensorType type) {
	if (!initialized) {
		kinc_acceleration_set_callback(accelerate);
		kinc_rotation_set_callback(rotate);
		initialized = true;
	}
	switch (type) {
	case SensorAccelerometer:
		return &accelerometer;
	case SensorGyroscope:
		return &gyroscope;
	default:
		return nullptr;
	}
}

Sensor::Sensor() : Changed(nullptr) {}
