#include "pch.h"

#include "VrPoseState.h"

VrPoseState::VrPoseState() : angularVelocity(Kore::vec3(0, 0, 0)), linearVelocity(Kore::vec3(0, 0, 0)),
							 angularAcceleration(Kore::vec3(0, 0, 0)), linearAcceleration(Kore::vec3(0, 0, 0)),
							 trackedDevice(HMD), isVisible(false), hmdPresenting(false), hmdMounted(false),
							 displayLost(false), shouldQuit(false), shouldRecenter(false)  {

}

VrPoseState::~VrPoseState() {

}
