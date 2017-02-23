#pragma once

#include <Kore/Vr/VrPoseState.h>

class SensorState {
public:
	SensorState();
	~SensorState();

	// Predicted pose configuration at requested absolute time.
	// One can determine the time difference between predicted and actual
	// readings by comparing ovrPoseState.TimeInSeconds.
	VrPoseState* predicted;

	// Actual recorded pose configuration based on the sensor sample at a 
	// moment closest to the requested time.
	VrPoseState* recorded;

	// Sensor temperature reading, in degrees Celsius, as sample time.
	float temperature;
	
	// Sensor status
	bool isVisible;
	bool hmdPresenting;
	bool hmdMounted;
	bool displayLost;
	bool shouldQuit;
	bool shouldRecenter;
};

