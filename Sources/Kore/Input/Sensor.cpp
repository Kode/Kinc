#include "pch.h"

#include "Sensor.h"

#include <Kinc/Input/Acceleration.h>
#include <Kinc/Input/Rotation.h>

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

Sensor* Sensor::the(SensorType type) {
	if (!initialized) {
		Kinc_AccelerationCallback = accelerate;
		Kinc_RotationCallback = rotate;
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
