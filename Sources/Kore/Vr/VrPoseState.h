#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Vr/VrPose.h>

enum TrackedDevice { HMD, Controller, ViveTracker };

class VrPoseState {
public:
	VrPoseState();
	~VrPoseState();

	VrPose vrPose;

	TrackedDevice trackedDevice;

	// Sensor status
	bool isVisible;
	bool hmdPresenting;
	bool hmdMounted;
	bool displayLost;
	bool shouldQuit;
	bool shouldRecenter;
};

