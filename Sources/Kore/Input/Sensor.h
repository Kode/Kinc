#pragma once

namespace Kore {
	enum SensorType { SensorAccelerometer, SensorGyroscope };

	class Sensor {
	public:
		static Sensor *the(SensorType type);
		void (*Changed)(float x, float y, float z);
		Sensor();
	};
}
