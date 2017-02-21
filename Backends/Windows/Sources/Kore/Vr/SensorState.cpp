#include "pch.h"

#include "SensorState.h"

SensorState::SensorState() : temperature(0), status(0) {
	predicted = new VrPoseState();
	recorded = new VrPoseState();
}


SensorState::~SensorState() {
	delete predicted;
	delete recorded;
}
