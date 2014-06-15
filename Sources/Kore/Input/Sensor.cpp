#include "pch.h"
#include "Sensor.h"

using namespace Kore;

namespace {
	Sensor accelerometer;
	Sensor gyroscope;
}

Sensor* Sensor::the(SensorType type) {
	switch (type) {
	case SensorAccelerometer:
		return &accelerometer;
	case SensorGyroscope:
		return &gyroscope;
	default:
		return nullptr;
	}
}

void Sensor::_changed(SensorType type, float x, float y, float z) {
	switch (type) {
	case SensorAccelerometer:
		if (accelerometer.Changed != nullptr) {
			accelerometer.Changed(x, y, z);
		}
		break;
	case SensorGyroscope:
		if (gyroscope.Changed != nullptr) {
			gyroscope.Changed(x, y, z);
		}
		break;
	}
}
