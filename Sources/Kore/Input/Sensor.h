#pragma once

namespace Kore {
	enum SensorType {
		SensorAccelerometer,
		SensorGyroscope
	};

	class Sensor {
	public:
		static Sensor* the(SensorType type);
		void (*Changed)(float x, float y, float z);
		
		//for backend
		static void _changed(SensorType type, float x, float y, float z);
	};
}
