#include "pch.h"

#include "VrPose.h"

VrPose::VrPose() : orientation(Kore::Quaternion(0, 0, 0, 1)), position(Kore::vec3(0, 0, 0)), left(0), right(0), bottom(0), top(0) {
}
