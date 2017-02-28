#pragma once

#include <Kore/Vr/VrPoseState.h>

class SensorState {
public:
	SensorState();
	~SensorState();

	VrPoseState* pose;
	
	// Sensor status
	bool isVisible;
	bool hmdPresenting;
	bool hmdMounted;
	bool displayLost;
	bool shouldQuit;
	bool shouldRecenter;
};

