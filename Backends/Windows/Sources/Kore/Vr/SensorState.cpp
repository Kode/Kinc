#include "pch.h"

#include "SensorState.h"

SensorState::SensorState() : temperature(0), isVisible(false), hmdPresenting(false), hmdMounted(false),
							 displayLost(false), shouldQuit(false), shouldRecenter(false) {
	predicted = new VrPoseState();
	recorded = new VrPoseState();
}


SensorState::~SensorState() {
	delete predicted;
	delete recorded;
}
