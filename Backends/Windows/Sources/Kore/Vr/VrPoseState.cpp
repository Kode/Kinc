#include "pch.h"

#include "VrPoseState.h"

VrPoseState::VrPoseState() : angularVelocity(Kore::vec3(0, 0, 0)), linearVelocity(Kore::vec3(0, 0, 0)),
							 angularAcceleration(Kore::vec3(0, 0, 0)), linearAcceleration(Kore::vec3(0, 0, 0)) {

	vrPose = new VrPose();
}


VrPoseState::~VrPoseState() {
	delete vrPose;
}


