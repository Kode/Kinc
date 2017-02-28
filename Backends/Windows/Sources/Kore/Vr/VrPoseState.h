#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Vr/VrPose.h>

class VrPoseState {
public:
	VrPoseState();
	~VrPoseState();

	VrPose* vrPose;
	Kore::vec3 angularVelocity;				// Angular velocity in radians per second.
	Kore::vec3 linearVelocity;				// Velocity in meters per second.
	Kore::vec3 angularAcceleration;			// Angular acceleration in radians per second per second.
	Kore::vec3 linearAcceleration;			// Acceleration in meters per second per second.

};

