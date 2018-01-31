#include "pch.h"

#include "VrPoseState.h"

VrPoseState::VrPoseState() : trackedDevice(HMD), isVisible(false), hmdPresenting(false), hmdMounted(false),
							 displayLost(false), shouldQuit(false), shouldRecenter(false)  {

}

VrPoseState::~VrPoseState() {

}
