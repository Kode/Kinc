#pragma once

#include <Kore/Vr/VrPoseState.h>

class SensorState {
public:
	SensorState();
	~SensorState();

	VrPoseState pose;
};

