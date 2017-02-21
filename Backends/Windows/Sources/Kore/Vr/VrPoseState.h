#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Vr/VrPose.h>

class VrPoseState {
public:
	VrPoseState();
	~VrPoseState();

	VrPose* vrPose;
	Kore::vec3 angularVelocity;
	Kore::vec3 linearVelocity;
	Kore::vec3 angularAcceleration;
	Kore::vec3 linearAcceleration;

};

