#pragma once

#include <Kore/Math/Quaternion.h>
#include <Kore/Math/Vector.h>

class VrPose {
public:
	VrPose();

	Kore::Quaternion orientation;
	Kore::vec3 position;
	
	// fov
	float left;
	float right;
	float bottom;
	float top;
};

